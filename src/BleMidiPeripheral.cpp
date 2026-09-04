#include "BleMidiPeripheral.h"

#include <BLE2902.h>
#include <BLEDevice.h>

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
  if (connectionStarted_) {
    connectionStarted_ = false;
    appState_.setBleConnectionState(BleConnectionState::Connected);
  }

  if (connectionEnded_) {
    connectionEnded_ = false;
    startAdvertising();
  }
}

void BleMidiPeripheral::onConnect(BLEServer* server) {
  (void)server;

  Serial.println("BLE: central connected");
  connectionStarted_ = true;
}

void BleMidiPeripheral::onDisconnect(BLEServer* server) {
  (void)server;

  Serial.println("BLE: central disconnected");
  connectionEnded_ = true;
}

void BleMidiPeripheral::createMidiService() {
  BLEService* midiService = server_->createService(BLE_MIDI_SERVICE_UUID);

  // The BLE MIDI characteristic is bidirectional. This slice only proves that
  // clients can discover/connect; the write handler for incoming MIDI packets
  // belongs in the next slice.
  midiDataCharacteristic_ = midiService->createCharacteristic(
      BLE_MIDI_DATA_CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_WRITE |
          BLECharacteristic::PROPERTY_WRITE_NR |
          BLECharacteristic::PROPERTY_NOTIFY);

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
