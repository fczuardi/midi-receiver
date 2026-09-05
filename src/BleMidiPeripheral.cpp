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
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();

  appState_.setBleConnectionState(BleConnectionState::Advertising);

  Serial.print("BLE MIDI: advertising as ");
  Serial.println(BLE_DEVICE_NAME);
}

void BleMidiPeripheral::update() {
  // MIDI.read() lets the FortySevenEffects MIDI parser consume bytes delivered
  // by the BLE-MIDI transport and call our note/real-time handlers.
  MIDI.read();

  if (connectionStarted_.exchange(false)) {
    appState_.setBleConnectionState(BleConnectionState::Connected);
  }

  if (connectionEnded_.exchange(false)) {
    appState_.setBleConnectionState(BleConnectionState::Advertising);
    appState_.clearActiveNotes();
  }

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

void BleMidiPeripheral::noteReceived(
    MidiActivityKind kind,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  PendingNoteEvent event;
  event.kind = kind;
  event.channel = channel;
  event.note = note;
  event.velocity = velocity;
  event.activityAtMs = millis();

  if (!enqueuePendingNoteEvent(event)) {
    droppedPendingNoteEventCount_.fetch_add(1);
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

  std::array<PendingNoteEvent, MAX_PENDING_NOTE_EVENTS> noteEvents{};
  size_t noteEventCount = 0;

  {
    std::lock_guard<std::mutex> lock(pendingNoteEventMutex_);
    noteEventCount = pendingNoteEventCount_;
    noteEvents = pendingNoteEvents_;
    pendingNoteEventCount_ = 0;
  }

  if (noteEventCount == 0) {
    return;
  }

  for (size_t index = 0; index < noteEventCount; ++index) {
    const PendingNoteEvent& event = noteEvents[index];
    appState_.recordMidiActivity(
        event.kind,
        event.channel,
        event.note,
        event.velocity,
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
    Serial.print(event.note);
    Serial.print(" velocity=");
    Serial.println(event.velocity);
  }

  const uint32_t droppedNoteEvents = droppedPendingNoteEventCount_.exchange(0);
  if (droppedNoteEvents > 0) {
    Serial.print("MIDI RX: dropped_note_events=");
    Serial.println(droppedNoteEvents);
  }
}

bool BleMidiPeripheral::enqueuePendingNoteEvent(const PendingNoteEvent& event) {
  std::lock_guard<std::mutex> lock(pendingNoteEventMutex_);
  if (pendingNoteEventCount_ >= pendingNoteEvents_.size()) {
    return false;
  }

  pendingNoteEvents_[pendingNoteEventCount_] = event;
  pendingNoteEventCount_ += 1;
  return true;
}
