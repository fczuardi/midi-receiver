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
  pendingNoteEventKind_.store(static_cast<uint8_t>(kind));
  pendingNoteChannel_.store(channel);
  pendingNoteNumber_.store(note);
  pendingNoteVelocity_.store(velocity);
  pendingMidiActivityAtMs_.store(millis());
  pendingNoteEventCount_.fetch_add(1);
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

  const uint32_t noteEventCount = pendingNoteEventCount_.exchange(0);
  if (noteEventCount == 0) {
    return;
  }

  const auto kind = static_cast<MidiActivityKind>(pendingNoteEventKind_.load());
  const uint8_t channel = pendingNoteChannel_.load();
  const uint8_t note = pendingNoteNumber_.load();
  const uint8_t velocity = pendingNoteVelocity_.load();
  const uint32_t activityAtMs = pendingMidiActivityAtMs_.load();

  for (uint32_t index = 0; index < noteEventCount; ++index) {
    appState_.recordMidiActivity(kind, channel, note, velocity, activityAtMs);
  }

  Serial.print("MIDI RX: ");
  Serial.print(kind == MidiActivityKind::NoteOn ? "note_on" : "note_off");
  Serial.print(" count=");
  Serial.print(noteEventCount);
  Serial.print(" total=");
  Serial.print(appState_.receivedMidiMessageCount());
  Serial.print(" channel=");
  Serial.print(channel);
  Serial.print(" note=");
  Serial.print(note);
  Serial.print(" velocity=");
  Serial.println(velocity);
}
