#include "BleMidiPeripheral.h"

#include <BLE2902.h>
#include <BLEDevice.h>

#include <algorithm>

namespace {
constexpr const char* BLE_DEVICE_NAME = "M5 BLE MIDI RX";
constexpr const char* BLE_MIDI_SERVICE_UUID = "03B80E5A-EDE8-4B33-A751-6CE34EC4C700";
constexpr const char* BLE_MIDI_DATA_CHARACTERISTIC_UUID = "7772E5DB-3868-4112-A1A9-F2669D106BF3";

constexpr uint16_t BLE_APPEARANCE_GENERIC_MEDIA_PLAYER = 0x0280;
}

BleMidiPeripheral::BleMidiPeripheral(AppState& appState)
    : appState_(appState) {
}

void BleMidiPeripheral::begin() {
  Serial.println("BLE: initializing device");

  BLEDevice::init(BLE_DEVICE_NAME);
  server_ = BLEDevice::createServer();
  server_->setCallbacks(this);

  createMidiService();
  startAdvertising();
}

void BleMidiPeripheral::update() {
  // BLE callbacks are owned by the ESP32 BLE stack. They can run at surprising
  // times, so callbacks below only record that an event happened. The main loop
  // calls this method, and this method performs the application state changes.
  if (connectionStarted_.exchange(false)) {
    appState_.setBleConnectionState(BleConnectionState::Connected);
  }

  if (connectionEnded_.exchange(false)) {
    startAdvertising();
  }

  applyPendingMidiPacketActivity();
}

void BleMidiPeripheral::onConnect(BLEServer* server) {
  (void)server;

  Serial.println("BLE: central connected");
  connectionStarted_.store(true);
}

void BleMidiPeripheral::onDisconnect(BLEServer* server) {
  (void)server;

  Serial.println("BLE: central disconnected");
  connectionEnded_.store(true);
}

void BleMidiPeripheral::onWrite(BLECharacteristic* characteristic) {
  const size_t packetSize = characteristic->getLength();
  const size_t capturedByteCount = std::min(packetSize, MAX_CAPTURED_MIDI_PACKET_BYTES);
  const uint8_t* packetBytes = characteristic->getData();

  // Keep the callback bounded: copy only a small debug snapshot, then let
  // update() handle app state and serial logging from the normal loop.
  std::lock_guard<std::mutex> lock(pendingMidiPacketMutex_);
  pendingLastMidiPacketSize_ = packetSize;
  pendingLastMidiPacketAtMs_ = millis();
  pendingCapturedMidiPacketByteCount_ = capturedByteCount;
  pendingMidiPacketWasTruncated_ = packetSize > MAX_CAPTURED_MIDI_PACKET_BYTES;
  std::copy(packetBytes, packetBytes + capturedByteCount, pendingCapturedMidiPacketBytes_.begin());
  pendingMidiPacketCount_ += 1;
}

void BleMidiPeripheral::createMidiService() {
  BLEService* midiService = server_->createService(BLE_MIDI_SERVICE_UUID);

  // The BLE MIDI characteristic is bidirectional. This slice records that raw
  // packets arrived, but intentionally leaves command parsing for later.
  midiDataCharacteristic_ = midiService->createCharacteristic(
      BLE_MIDI_DATA_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_WRITE_NR |
          BLECharacteristic::PROPERTY_NOTIFY);

  midiDataCharacteristic_->setCallbacks(this);
  midiDataCharacteristic_->addDescriptor(new BLE2902());
  midiService->start();

  Serial.println("BLE: MIDI service started");
}

void BleMidiPeripheral::startAdvertising() {
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(BLE_MIDI_SERVICE_UUID);
  advertising->setAppearance(BLE_APPEARANCE_GENERIC_MEDIA_PLAYER);
  advertising->setScanResponse(true);
  advertising->setMinPreferred(0x06);
  advertising->setMaxPreferred(0x12);

  BLEDevice::startAdvertising();
  appState_.setBleConnectionState(BleConnectionState::Advertising);

  Serial.print("BLE: advertising as ");
  Serial.println(BLE_DEVICE_NAME);
}

void BleMidiPeripheral::applyPendingMidiPacketActivity() {
  uint32_t packetCount = 0;
  size_t lastPacketSize = 0;
  uint32_t lastPacketAtMs = 0;
  size_t capturedByteCount = 0;
  bool packetWasTruncated = false;
  std::array<uint8_t, MAX_CAPTURED_MIDI_PACKET_BYTES> capturedPacketBytes{};

  {
    std::lock_guard<std::mutex> lock(pendingMidiPacketMutex_);
    packetCount = pendingMidiPacketCount_;
    lastPacketSize = pendingLastMidiPacketSize_;
    lastPacketAtMs = pendingLastMidiPacketAtMs_;
    capturedByteCount = pendingCapturedMidiPacketByteCount_;
    packetWasTruncated = pendingMidiPacketWasTruncated_;
    capturedPacketBytes = pendingCapturedMidiPacketBytes_;
    pendingMidiPacketCount_ = 0;
  }

  if (packetCount == 0) {
    return;
  }

  for (uint32_t packetIndex = 0; packetIndex < packetCount; ++packetIndex) {
    appState_.recordMidiPacketActivity(lastPacketSize, lastPacketAtMs);
  }

  Serial.print("MIDI RX: packets=");
  Serial.print(packetCount);
  Serial.print(" total=");
  Serial.print(appState_.receivedMidiPacketCount());
  Serial.print(" last_bytes=");
  Serial.print(appState_.lastMidiPacketSize());
  Serial.print(" data=");
  printLastMidiPacketBytes(capturedPacketBytes.data(), capturedByteCount, packetWasTruncated);
}

void BleMidiPeripheral::printLastMidiPacketBytes(
    const uint8_t* packetBytes,
    size_t byteCount,
    bool wasTruncated) const {
  for (size_t byteIndex = 0; byteIndex < byteCount; ++byteIndex) {
    if (byteIndex > 0) {
      Serial.print(' ');
    }

    if (packetBytes[byteIndex] < 0x10) {
      Serial.print('0');
    }
    Serial.print(packetBytes[byteIndex], HEX);
  }

  if (wasTruncated) {
    Serial.print(" ...");
  }

  Serial.println();
}
