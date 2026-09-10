#include "BleMidiInput.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "MidiNoteEventFactory.h"

namespace {
#ifndef BLE_MIDI_DEVICE_NAME
#define BLE_MIDI_DEVICE_NAME "M5 BLE MIDI RX"
#endif

constexpr const char* BLE_DEVICE_NAME = BLE_MIDI_DEVICE_NAME;
constexpr const char* MIDI_SERVICE_UUID = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";
constexpr const char* MIDI_CHARACTERISTIC_UUID = "7772E5DB-3868-4112-A1A9-F2669D106BF3";

int normalizePitchBend(uint16_t bendValue) {
  return static_cast<int>(bendValue) - 8192;
}

}

class BleMidiServerCallbacks : public NimBLEServerCallbacks {
public:
  void onConnect(NimBLEServer*, NimBLEConnInfo&) override {
    BleMidiInput::handleConnected();
  }

  void onDisconnect(NimBLEServer* server, NimBLEConnInfo&, int) override {
    BleMidiInput::handleDisconnected();
    server->startAdvertising();
  }
};

class BleMidiCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
public:
  void onWrite(NimBLECharacteristic* characteristic, NimBLEConnInfo&) override {
    std::string value = characteristic->getValue();
    if (value.empty() || BleMidiInput::activeInstance_ == nullptr) {
      return;
    }

    BleMidiInput::activeInstance_->parseBleMidiPacket(
        reinterpret_cast<uint8_t*>(&value[0]),
        value.size());
  }
};

namespace {
BleMidiServerCallbacks serverCallbacks;
BleMidiCharacteristicCallbacks characteristicCallbacks;
}

BleMidiInput* BleMidiInput::activeInstance_ = nullptr;

void BleMidiInput::begin() {
  activeInstance_ = this;

  NimBLEDevice::init(BLE_DEVICE_NAME);

  NimBLEServer* server = NimBLEDevice::createServer();
  server->setCallbacks(&serverCallbacks);
  server->advertiseOnDisconnect(true);

  NimBLEService* service = server->createService(NimBLEUUID(MIDI_SERVICE_UUID));
  NimBLECharacteristic* characteristic = service->createCharacteristic(
      NimBLEUUID(MIDI_CHARACTERISTIC_UUID),
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::NOTIFY |
          NIMBLE_PROPERTY::WRITE_NR);
  characteristic->setCallbacks(&characteristicCallbacks);

  NimBLEAdvertising* advertising = server->getAdvertising();
  advertising->addServiceUUID(service->getUUID());
  advertising->enableScanResponse(true);
  advertising->setName(BLE_DEVICE_NAME);
  advertising->setAppearance(0x00);
  advertising->start();

  if (diagnosticSink_ != nullptr) {
    diagnosticSink_->onBleMidiAdvertising(BLE_DEVICE_NAME);
  }
}

void BleMidiInput::update() {
  // Handle disconnect before reading more MIDI. If the sender disappeared in
  // the middle of a chord, pending Note On events must not recreate held notes
  // after consumers clear them.
  if (connectionEnded_.exchange(false)) {
    connected_ = false;
    discardPendingMidiActivity();

    if (diagnosticSink_ != nullptr) {
      diagnosticSink_->onBleMidiDisconnected();
    }

    if (instrumentEventSink_ != nullptr) {
      instrumentEventSink_->onDisconnected();
    }
  }

  if (connectionStarted_.exchange(false)) {
    connected_ = true;

    if (diagnosticSink_ != nullptr) {
      diagnosticSink_->onBleMidiConnected();
    }
  }

  if (!connected_) {
    discardPendingMidiActivity();
    return;
  }

  applyPendingMidiActivity();
}

void BleMidiInput::setInstrumentEventSink(InstrumentEventSink* sink) {
  instrumentEventSink_ = sink;
}

void BleMidiInput::setDiagnosticSink(BleMidiInputDiagnosticSink* sink) {
  diagnosticSink_ = sink;
}

void BleMidiInput::handleConnected() {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->connectionStarted_.store(true);
}

void BleMidiInput::handleDisconnected() {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->connectionEnded_.store(true);
}

void BleMidiInput::noteReceived(
    PendingMidiEventKind kind,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  PendingMidiEvent event;
  event.kind = kind;
  event.channel = channel;
  event.data1 = note;
  event.data2 = velocity;
  event.activityAtMs = millis();

  if (!enqueuePendingMidiEvent(event)) {
    droppedPendingMidiEventCount_.fetch_add(1);
  }
}

void BleMidiInput::controlChangeReceived(
    uint8_t channel,
    uint8_t controllerNumber,
    uint8_t controllerValue) {
  PendingMidiEvent event;
  event.kind = PendingMidiEventKind::ControlChange;
  event.channel = channel;
  event.data1 = controllerNumber;
  event.data2 = controllerValue;
  event.activityAtMs = millis();

  if (!enqueuePendingMidiEvent(event)) {
    droppedPendingMidiEventCount_.fetch_add(1);
  }
}

void BleMidiInput::pitchBendReceived(uint8_t channel, int bendValue) {
  PendingMidiEvent event;
  event.kind = PendingMidiEventKind::PitchBend;
  event.channel = channel;
  event.bendValue = bendValue;
  event.activityAtMs = millis();

  if (!enqueuePendingMidiEvent(event)) {
    droppedPendingMidiEventCount_.fetch_add(1);
  }
}

void BleMidiInput::parseBleMidiPacket(uint8_t* data, size_t size) {
  if (size < 3 || (data[0] & 0x80) == 0 || (data[1] & 0x80) == 0) {
    return;
  }

  uint8_t* cursor = data + 1;
  uint8_t* end = data + size;
  uint8_t runningStatus = 0;

  while (cursor < end) {
    if ((*cursor & 0x80) != 0) {
      cursor += 1;
    }

    if (cursor >= end) {
      return;
    }

    if ((*cursor & 0x80) != 0) {
      runningStatus = *cursor;
      cursor += 1;
    }

    if (runningStatus == 0 || !parseMidiMessage(runningStatus, cursor, end)) {
      return;
    }
  }
}

bool BleMidiInput::parseMidiMessage(uint8_t status, uint8_t*& cursor, uint8_t* end) {
  const uint8_t command = status >> 4;
  const uint8_t channel = status & 0x0f;

  switch (command) {
    case 0x8:
    case 0x9: {
      if (end - cursor < 2) {
        return false;
      }

      const uint8_t note = cursor[0];
      const uint8_t velocity = cursor[1];
      cursor += 2;
      noteReceived(
          command == 0x9 ? PendingMidiEventKind::NoteOn
                         : PendingMidiEventKind::NoteOff,
          channel,
          note,
          velocity);
      return true;
    }

    case 0xb: {
      if (end - cursor < 2) {
        return false;
      }

      const uint8_t controllerNumber = cursor[0];
      const uint8_t controllerValue = cursor[1];
      cursor += 2;
      controlChangeReceived(channel, controllerNumber, controllerValue);
      return true;
    }

    case 0xe: {
      if (end - cursor < 2) {
        return false;
      }

      const uint16_t bendValue =
          (static_cast<uint16_t>(cursor[1] & 0x7f) << 7) |
          static_cast<uint16_t>(cursor[0] & 0x7f);
      cursor += 2;
      pitchBendReceived(channel, normalizePitchBend(bendValue));
      return true;
    }

    case 0xa:
    case 0xc:
    case 0xd:
      return false;

    default:
      return false;
  }
}

void BleMidiInput::applyPendingMidiActivity() {
  std::array<PendingMidiEvent, MAX_PENDING_MIDI_EVENTS> midiEvents{};
  size_t midiEventCount = 0;

  {
    std::lock_guard<std::mutex> lock(pendingMidiEventMutex_);
    midiEventCount = pendingMidiEventCount_;
    midiEvents = pendingMidiEvents_;
    pendingMidiEventCount_ = 0;
  }

  for (size_t index = 0; index < midiEventCount; ++index) {
    const PendingMidiEvent& event = midiEvents[index];

    if (event.kind == PendingMidiEventKind::ControlChange) {
      if (diagnosticSink_ != nullptr) {
        diagnosticSink_->onBleMidiControlChange(
            event.channel,
            event.data1,
            event.data2,
            event.activityAtMs);
      }
    } else if (event.kind == PendingMidiEventKind::PitchBend) {
      const PitchBendEvent pitchBendEvent = {
          event.channel,
          static_cast<int16_t>(event.bendValue),
      };

      if (diagnosticSink_ != nullptr) {
        diagnosticSink_->onBleMidiPitchBend(
            event.channel,
            event.bendValue,
            event.activityAtMs);
      }

      if (instrumentEventSink_ != nullptr) {
        instrumentEventSink_->onPitchBendEvent(pitchBendEvent);
      }
    } else if (
        event.kind == PendingMidiEventKind::NoteOn ||
        event.kind == PendingMidiEventKind::NoteOff) {
      const NoteEvent noteEvent = makeNoteEvent(
          event.kind == PendingMidiEventKind::NoteOn ? NoteEventType::NoteOn
                                                     : NoteEventType::NoteOff,
          event.channel,
          event.data1,
          event.data2);

      if (diagnosticSink_ != nullptr) {
        diagnosticSink_->onBleMidiNoteEvent(noteEvent, event.activityAtMs);
      }

      if (instrumentEventSink_ != nullptr) {
        instrumentEventSink_->onNoteEvent(noteEvent);
      }
    }
  }

  const uint32_t droppedMidiEvents = droppedPendingMidiEventCount_.exchange(0);
  if (droppedMidiEvents > 0 && diagnosticSink_ != nullptr) {
    diagnosticSink_->onBleMidiDroppedEvents(droppedMidiEvents);
  }
}

void BleMidiInput::discardPendingMidiActivity() {
  droppedPendingMidiEventCount_.store(0);

  std::lock_guard<std::mutex> lock(pendingMidiEventMutex_);
  pendingMidiEventCount_ = 0;
}

bool BleMidiInput::enqueuePendingMidiEvent(const PendingMidiEvent& event) {
  std::lock_guard<std::mutex> lock(pendingMidiEventMutex_);
  if (pendingMidiEventCount_ >= pendingMidiEvents_.size()) {
    return false;
  }

  pendingMidiEvents_[pendingMidiEventCount_] = event;
  pendingMidiEventCount_ += 1;
  return true;
}
