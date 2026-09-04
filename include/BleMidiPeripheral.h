#pragma once

#include <BLECharacteristic.h>
#include <BLEServer.h>

#include <array>
#include <atomic>
#include <mutex>

#include "AppState.h"

// Owns BLE setup for this firmware.
//
// For now this class advertises the BLE MIDI service and reports raw packet
// activity. Parsing MIDI commands will be added as a separate, reviewable slice.
class BleMidiPeripheral : private BLEServerCallbacks, private BLECharacteristicCallbacks {
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
  void onWrite(BLECharacteristic* characteristic) override;

  void createMidiService();
  void startAdvertising();
  void applyPendingMidiPacketActivity();
  void printLastMidiPacketBytes(const uint8_t* packetBytes, size_t byteCount, bool wasTruncated) const;

  AppState& appState_;
  BLEServer* server_ = nullptr;
  BLECharacteristic* midiDataCharacteristic_ = nullptr;
  std::atomic<bool> connectionStarted_{false};
  std::atomic<bool> connectionEnded_{false};

  static constexpr size_t MAX_CAPTURED_MIDI_PACKET_BYTES = 32;

  std::mutex pendingMidiPacketMutex_;
  uint32_t pendingMidiPacketCount_ = 0;
  size_t pendingLastMidiPacketSize_ = 0;
  uint32_t pendingLastMidiPacketAtMs_ = 0;
  size_t pendingCapturedMidiPacketByteCount_ = 0;
  bool pendingMidiPacketWasTruncated_ = false;
  std::array<uint8_t, MAX_CAPTURED_MIDI_PACKET_BYTES> pendingCapturedMidiPacketBytes_{};
};
