#include "BleMidiPeripheral.h"

#include <BLEMIDI_Transport.h>
#include <hardware/BLEMIDI_ESP32.h>

namespace {
constexpr const char* BLE_DEVICE_NAME = "M5 BLE MIDI RX";
}

BLEMIDI_CREATE_INSTANCE("M5 BLE MIDI RX", MIDI)

BleMidiPeripheral* BleMidiPeripheral::activeInstance_ = nullptr;

BleMidiPeripheral::BleMidiPeripheral(AppState& appState)
    : appState_(appState) {
}

void BleMidiPeripheral::begin() {
  Serial.println("BLE MIDI: initializing library transport");

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

  appState_.setBleConnectionState(BleConnectionState::Advertising);

  Serial.print("BLE MIDI: advertising as ");
  Serial.println(BLE_DEVICE_NAME);
}

void BleMidiPeripheral::update() {
  // Handle disconnect before reading more MIDI. If the sender disappeared in
  // the middle of a chord, pending Note On events must not recreate held notes
  // after AppState clears them.
  if (connectionEnded_.exchange(false)) {
    connected_ = false;
    discardPendingMidiActivity();
    appState_.setBleConnectionState(BleConnectionState::Advertising);
    appState_.clearActiveNotes();
  }

  if (connectionStarted_.exchange(false)) {
    connected_ = true;
    appState_.setBleConnectionState(BleConnectionState::Connected);
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

void BleMidiPeripheral::handleConnected() {
  if (activeInstance_ == nullptr) {
    return;
  }

  Serial.println("BLE MIDI: central connected");
  activeInstance_->connectionStarted_.store(true);
}

void BleMidiPeripheral::handleDisconnected() {
  if (activeInstance_ == nullptr) {
    return;
  }

  Serial.println("BLE MIDI: central disconnected");
  activeInstance_->connectionEnded_.store(true);
}

void BleMidiPeripheral::handleActiveSensing() {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->activeSensingReceived();
}

void BleMidiPeripheral::handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->noteReceived(MidiActivityKind::NoteOn, channel, note, velocity);
}

void BleMidiPeripheral::handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->noteReceived(MidiActivityKind::NoteOff, channel, note, velocity);
}

void BleMidiPeripheral::handleControlChange(
    uint8_t channel,
    uint8_t controllerNumber,
    uint8_t controllerValue) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->controlChangeReceived(channel, controllerNumber, controllerValue);
}

void BleMidiPeripheral::handlePitchBend(uint8_t channel, int bendValue) {
  if (activeInstance_ == nullptr) {
    return;
  }

  activeInstance_->pitchBendReceived(channel, bendValue);
}

void BleMidiPeripheral::noteReceived(
    MidiActivityKind kind,
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

void BleMidiPeripheral::controlChangeReceived(
    uint8_t channel,
    uint8_t controllerNumber,
    uint8_t controllerValue) {
  PendingMidiEvent event;
  event.kind = MidiActivityKind::ControlChange;
  event.channel = channel;
  event.data1 = controllerNumber;
  event.data2 = controllerValue;
  event.activityAtMs = millis();

  if (!enqueuePendingMidiEvent(event)) {
    droppedPendingMidiEventCount_.fetch_add(1);
  }
}

void BleMidiPeripheral::pitchBendReceived(uint8_t channel, int bendValue) {
  PendingMidiEvent event;
  event.kind = MidiActivityKind::PitchBend;
  event.channel = channel;
  event.bendValue = bendValue;
  event.activityAtMs = millis();

  if (!enqueuePendingMidiEvent(event)) {
    droppedPendingMidiEventCount_.fetch_add(1);
  }
}

void BleMidiPeripheral::activeSensingReceived() {
  pendingMidiActivityAtMs_.store(millis());
  pendingActiveSensingCount_.fetch_add(1);
}

void BleMidiPeripheral::applyPendingMidiActivity() {
  const uint32_t activeSensingCount = pendingActiveSensingCount_.exchange(0);
  for (uint32_t index = 0; index < activeSensingCount; ++index) {
    appState_.recordMidiActivity(
        MidiActivityKind::ActiveSensing,
        0,
        0,
        0,
        pendingMidiActivityAtMs_.load());
  }

  if (activeSensingCount > 0) {
    Serial.print("MIDI RX: active_sensing=");
    Serial.print(activeSensingCount);
    Serial.print(" total=");
    Serial.println(appState_.receivedMidiMessageCount());
  }

  std::array<PendingMidiEvent, MAX_PENDING_MIDI_EVENTS> midiEvents{};
  size_t midiEventCount = 0;

  {
    std::lock_guard<std::mutex> lock(pendingMidiEventMutex_);
    midiEventCount = pendingMidiEventCount_;
    midiEvents = pendingMidiEvents_;
    pendingMidiEventCount_ = 0;
  }

  if (midiEventCount == 0) {
    return;
  }

  for (size_t index = 0; index < midiEventCount; ++index) {
    const PendingMidiEvent& event = midiEvents[index];

    if (event.kind == MidiActivityKind::ControlChange) {
      appState_.recordControlChange(
          event.channel,
          event.data1,
          event.data2,
          event.activityAtMs);

      Serial.print("MIDI RX: control_change total=");
      Serial.print(appState_.receivedMidiMessageCount());
      Serial.print(" channel=");
      Serial.print(event.channel);
      Serial.print(" cc=");
      Serial.print(event.data1);
      Serial.print(" value=");
      Serial.println(event.data2);
    } else if (event.kind == MidiActivityKind::PitchBend) {
      appState_.recordPitchBend(event.channel, event.bendValue, event.activityAtMs);

      Serial.print("MIDI RX: pitch_bend total=");
      Serial.print(appState_.receivedMidiMessageCount());
      Serial.print(" channel=");
      Serial.print(event.channel);
      Serial.print(" value=");
      Serial.println(event.bendValue);
    } else {
      appState_.recordMidiActivity(
          event.kind,
          event.channel,
          event.data1,
          event.data2,
          event.activityAtMs);

      Serial.print("MIDI RX: ");
      Serial.print(event.kind == MidiActivityKind::NoteOn ? "note_on" : "note_off");
      Serial.print(" total=");
      Serial.print(appState_.receivedMidiMessageCount());
      Serial.print(" active=");
      Serial.print(appState_.activeNoteCount());
      Serial.print(" channel=");
      Serial.print(event.channel);
      Serial.print(" note=");
      Serial.print(event.data1);
      Serial.print(" velocity=");
      Serial.println(event.data2);
    }
  }

  const uint32_t droppedMidiEvents = droppedPendingMidiEventCount_.exchange(0);
  if (droppedMidiEvents > 0) {
    Serial.print("MIDI RX: dropped_midi_events=");
    Serial.println(droppedMidiEvents);
  }
}

void BleMidiPeripheral::discardPendingMidiActivity() {
  pendingActiveSensingCount_.store(0);
  pendingMidiActivityAtMs_.store(0);
  droppedPendingMidiEventCount_.store(0);

  std::lock_guard<std::mutex> lock(pendingMidiEventMutex_);
  pendingMidiEventCount_ = 0;
}

bool BleMidiPeripheral::enqueuePendingMidiEvent(const PendingMidiEvent& event) {
  std::lock_guard<std::mutex> lock(pendingMidiEventMutex_);
  if (pendingMidiEventCount_ >= pendingMidiEvents_.size()) {
    return false;
  }

  pendingMidiEvents_[pendingMidiEventCount_] = event;
  pendingMidiEventCount_ += 1;
  return true;
}
