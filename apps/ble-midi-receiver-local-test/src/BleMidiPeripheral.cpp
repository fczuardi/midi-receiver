#include "BleMidiPeripheral.h"

#include <Arduino.h>

BleMidiPeripheral::BleMidiPeripheral(AppState& appState)
    : appState_(appState) {
}

void BleMidiPeripheral::begin() {
  Serial.println("BLE MIDI: initializing library transport");

  input_.setDiagnosticSink(this);
  input_.begin();
}

void BleMidiPeripheral::update() {
  input_.update();
}

void BleMidiPeripheral::setInstrumentEventSink(InstrumentEventSink* sink) {
  input_.setInstrumentEventSink(sink);
}

void BleMidiPeripheral::onBleMidiAdvertising(const char* deviceName) {
  appState_.setBleConnectionState(BleConnectionState::Advertising);

  Serial.print("BLE MIDI: advertising as ");
  Serial.println(deviceName);
}

void BleMidiPeripheral::onBleMidiConnected() {
  Serial.println("BLE MIDI: central connected");
  appState_.setBleConnectionState(BleConnectionState::Connected);
}

void BleMidiPeripheral::onBleMidiDisconnected() {
  Serial.println("BLE MIDI: central disconnected");
  appState_.setBleConnectionState(BleConnectionState::Advertising);
  appState_.clearActiveNotes();
}

void BleMidiPeripheral::onBleMidiNoteEvent(
    const NoteEvent& event,
    uint32_t activityAtMs) {
  appState_.recordNoteEvent(event, activityAtMs);

  Serial.print("MIDI RX: note_event type=");
  Serial.print(event.type == NoteEventType::NoteOn ? "note_on" : "note_off");
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

void BleMidiPeripheral::onBleMidiControlChange(
    uint8_t channel,
    uint8_t controllerNumber,
    uint8_t controllerValue,
    uint32_t activityAtMs) {
  appState_.recordControlChange(
      channel,
      controllerNumber,
      controllerValue,
      activityAtMs);

  Serial.print("MIDI RX: control_change total=");
  Serial.print(appState_.receivedMidiMessageCount());
  Serial.print(" channel=");
  Serial.print(channel);
  Serial.print(" cc=");
  Serial.print(controllerNumber);
  Serial.print(" value=");
  Serial.println(controllerValue);
}

void BleMidiPeripheral::onBleMidiPitchBend(
    uint8_t channel,
    int bendValue,
    uint32_t activityAtMs) {
  appState_.recordPitchBend(channel, bendValue, activityAtMs);

  Serial.print("MIDI RX: pitch_bend total=");
  Serial.print(appState_.receivedMidiMessageCount());
  Serial.print(" channel=");
  Serial.print(channel);
  Serial.print(" value=");
  Serial.println(bendValue);
}

void BleMidiPeripheral::onBleMidiDroppedEvents(uint32_t droppedEventCount) {
  Serial.print("MIDI RX: dropped_midi_events=");
  Serial.println(droppedEventCount);
}
