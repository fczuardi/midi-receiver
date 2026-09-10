#include "BleMidiInput.h"

#include <Arduino.h>
#include <BLEMIDI_Transport.h>
#if defined(BLE_MIDI_INPUT_USE_CLASSIC_ESP32_BLE)
#include <hardware/BLEMIDI_ESP32.h>
#else
#include <NimBLEDevice.h>
#if defined(NIMBLE_CPP_VERSION) && \
    NIMBLE_CPP_VERSION >= NIMBLE_CPP_VERSION_VAL(2, 0, 0)
#ifndef ESP_LE_AUTH_BOND
#define ESP_LE_AUTH_BOND 0x01
#endif
// BLE-MIDI 2.2 uses the NimBLE-Arduino 1.x security wrapper. NimBLE 2.x moved
// this operation to NimBLEDevice, so keep the transport source compatible here.
class NimBLESecurity {
 public:
  void setAuthenticationMode(uint8_t auth) {
    NimBLEDevice::setSecurityAuth(auth);
  }
};
#endif
#include "hardware/BLEMIDI_ESP32_NimBLE.h"
#endif

#include "MidiNoteEventFactory.h"

namespace {
// The BLE-MIDI library creates the advertised device through a static macro.
#ifndef BLE_MIDI_DEVICE_NAME
#define BLE_MIDI_DEVICE_NAME "M5 BLE MIDI RX"
#endif

constexpr const char* BLE_DEVICE_NAME = BLE_MIDI_DEVICE_NAME;
}

BLEMIDI_CREATE_INSTANCE(BLE_MIDI_DEVICE_NAME, MIDI)

BleMidiInput* BleMidiInput::activeInstance_ = nullptr;

void BleMidiInput::begin() {
  activeInstance_ = this;

  BLEMIDI.setHandleConnected(handleConnected);
  BLEMIDI.setHandleDisconnected(handleDisconnected);

  MIDI.setHandleActiveSensing(handleActiveSensing);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleControlChange);
  MIDI.setHandlePitchBend(handlePitchBend);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

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

  // MIDI.read() lets the FortySevenEffects MIDI parser consume bytes delivered
  // by the BLE-MIDI transport and call our note/real-time handlers.
  MIDI.read();
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

void BleMidiInput::handleActiveSensing() {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->activeSensingReceived();
}

void BleMidiInput::handleNoteOn(
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->noteReceived(
      PendingMidiEventKind::NoteOn,
      channel,
      note,
      velocity);
}

void BleMidiInput::handleNoteOff(
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->noteReceived(
      PendingMidiEventKind::NoteOff,
      channel,
      note,
      velocity);
}

void BleMidiInput::handleControlChange(
    uint8_t channel,
    uint8_t controllerNumber,
    uint8_t controllerValue) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->controlChangeReceived(
      channel,
      controllerNumber,
      controllerValue);
}

void BleMidiInput::handlePitchBend(uint8_t channel, int bendValue) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->pitchBendReceived(channel, bendValue);
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

void BleMidiInput::activeSensingReceived() {
  pendingMidiActivityAtMs_.store(millis());
  pendingActiveSensingCount_.fetch_add(1);
}

void BleMidiInput::applyPendingMidiActivity() {
  const uint32_t activeSensingCount = pendingActiveSensingCount_.exchange(0);
  if (activeSensingCount > 0 && diagnosticSink_ != nullptr) {
    diagnosticSink_->onBleMidiActiveSensing(
        activeSensingCount,
        pendingMidiActivityAtMs_.load());
  }

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
  pendingActiveSensingCount_.store(0);
  pendingMidiActivityAtMs_.store(0);
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
