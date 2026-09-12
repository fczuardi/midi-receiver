#pragma once

#include "AppState.h"
#include "BleMidiInput.h"

// Firmware-facing BLE MIDI receiver with display and serial diagnostics.
//
// BleMidiInput owns transport and event production. This class observes that
// input and keeps the receiver-specific AppState/logging behavior.
class BleMidiPeripheral : public BleMidiInputDiagnosticSink {
public:
  explicit BleMidiPeripheral(AppState& appState);

  // Initialize the BLE stack, create the MIDI GATT service, and advertise it.
  void begin();

  // Apply BLE events that were observed by callbacks.
  // Keeping state changes here makes the main loop easier to inspect.
  void update();

  // Register an optional instrument-event consumer.
  // The sink is not owned by BleMidiPeripheral and must outlive it.
  void setInstrumentEventSink(InstrumentEventSink* sink);

private:
  void onBleMidiAdvertising(const char* deviceName) override;
  void onBleMidiConnected() override;
  void onBleMidiDisconnected() override;
  void onBleMidiNoteEvent(
      const NoteEvent& event,
      uint32_t activityAtMs) override;
  void onBleMidiControlChange(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue,
      uint32_t activityAtMs) override;
  void onBleMidiPitchBend(
      uint8_t channel,
      int bendValue,
      uint32_t activityAtMs) override;
  void onBleMidiUnsupportedMessage(
      const UnsupportedMidiMessage& message) override;
  void onBleMidiDroppedEvents(uint32_t droppedEventCount) override;

  AppState& appState_;
  BleMidiInput input_;
};
