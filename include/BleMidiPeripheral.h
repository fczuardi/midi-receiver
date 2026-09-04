#pragma once

#include <BLECharacteristic.h>
#include <BLEServer.h>

#include "AppState.h"

// Owns BLE setup for this firmware.
//
// For now this class only advertises the BLE MIDI service and tracks whether a
// central device is connected. Receiving and parsing MIDI bytes will be added
// as a separate, reviewable slice.
class BleMidiPeripheral : private BLEServerCallbacks {
public:
  explicit BleMidiPeripheral(AppState& appState);

  // Initialize the BLE stack, create the MIDI GATT service, and advertise it.
  void begin();

  // Apply BLE events that were observed by callbacks.
  // Keeping state changes here makes the main loop easier to inspect.
  void update();

private:
  void onConnect(BLEServer* server) override;
  void onDisconnect(BLEServer* server) override;

  void createMidiService();
  void startAdvertising();

  AppState& appState_;
  BLEServer* server_ = nullptr;
  BLECharacteristic* midiDataCharacteristic_ = nullptr;
  volatile bool connectionStarted_ = false;
  volatile bool connectionEnded_ = false;
};
