#pragma once

// Compatibility copy of BLE-MIDI 2.2's ESP32 NimBLE backend.
//
// BLE-MIDI 2.2 still uses NimBLE-Arduino 1.x callback signatures, while
// NimBLE-Arduino 2.x adds NimBLEConnInfo parameters to server and
// characteristic callbacks. Keeping this backend local lets BleMidiInput remain
// plug-and-play for pioarduino/NimBLE 2.x consumers without patching generated
// PlatformIO dependencies.

#include <NimBLEDevice.h>

BEGIN_BLEMIDI_NAMESPACE

class BLEMIDI_ESP32_NimBLE {
 private:
  BLEServer* _server = nullptr;
  BLEAdvertising* _advertising = nullptr;
  BLECharacteristic* _characteristic = nullptr;

  BLEMIDI_Transport<class BLEMIDI_ESP32_NimBLE>* _bleMidiTransport = nullptr;

  friend class MyServerCallbacks;
  friend class MyCharacteristicCallbacks;

 protected:
  QueueHandle_t mRxQueue;

 public:
  BLEMIDI_ESP32_NimBLE() {}

  bool begin(
      const char*,
      BLEMIDI_Transport<class BLEMIDI_ESP32_NimBLE>*);

  void end() {}

  void write(uint8_t* buffer, size_t length) {
    _characteristic->setValue(buffer, length);
    _characteristic->notify();
  }

  bool available(byte* pvBuffer) {
    return xQueueReceive(mRxQueue, static_cast<void*>(pvBuffer), 0);
  }

  void add(byte value) {
    xQueueSend(mRxQueue, &value, portMAX_DELAY);
  }

 protected:
  void receive(uint8_t* buffer, size_t length) {
    _bleMidiTransport->receive(buffer, length);
  }

  void connected() {
    if (_bleMidiTransport->_connectedCallback) {
      _bleMidiTransport->_connectedCallback();
    }
  }

  void disconnected() {
    if (_bleMidiTransport->_disconnectedCallback) {
      _bleMidiTransport->_disconnectedCallback();
    }
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
 public:
  explicit MyServerCallbacks(BLEMIDI_ESP32_NimBLE* bluetoothEsp32)
      : _bluetoothEsp32(bluetoothEsp32) {}

 protected:
  BLEMIDI_ESP32_NimBLE* _bluetoothEsp32 = nullptr;

#if defined(NIMBLE_CPP_VERSION) && \
    NIMBLE_CPP_VERSION >= NIMBLE_CPP_VERSION_VAL(2, 0, 0)
  void onConnect(BLEServer*, BLEConnInfo&) override {
    if (_bluetoothEsp32) {
      _bluetoothEsp32->connected();
    }
  }

  void onDisconnect(BLEServer*, BLEConnInfo&, int) override {
    if (_bluetoothEsp32) {
      _bluetoothEsp32->disconnected();
    }
  }
#else
  void onConnect(BLEServer*) override {
    if (_bluetoothEsp32) {
      _bluetoothEsp32->connected();
    }
  }

  void onDisconnect(BLEServer*) override {
    if (_bluetoothEsp32) {
      _bluetoothEsp32->disconnected();
    }
  }
#endif
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
 public:
  explicit MyCharacteristicCallbacks(BLEMIDI_ESP32_NimBLE* bluetoothEsp32)
      : _bluetoothEsp32(bluetoothEsp32) {}

 protected:
  BLEMIDI_ESP32_NimBLE* _bluetoothEsp32 = nullptr;

  void receiveWrite(BLECharacteristic* characteristic) {
    std::string rxValue = characteristic->getValue();
    if (!rxValue.empty()) {
      _bluetoothEsp32->receive(
          reinterpret_cast<uint8_t*>(&rxValue[0]),
          rxValue.length());
    }
  }

#if defined(NIMBLE_CPP_VERSION) && \
    NIMBLE_CPP_VERSION >= NIMBLE_CPP_VERSION_VAL(2, 0, 0)
  void onWrite(BLECharacteristic* characteristic, BLEConnInfo&) override {
    receiveWrite(characteristic);
  }
#else
  void onWrite(BLECharacteristic* characteristic) override {
    receiveWrite(characteristic);
  }
#endif
};

bool BLEMIDI_ESP32_NimBLE::begin(
    const char* deviceName,
    BLEMIDI_Transport<class BLEMIDI_ESP32_NimBLE>* bleMidiTransport) {
  _bleMidiTransport = bleMidiTransport;

  BLEDevice::init(deviceName);

  mRxQueue = xQueueCreate(64, sizeof(uint8_t));

  _server = BLEDevice::createServer();
  _server->setCallbacks(new MyServerCallbacks(this));
  _server->advertiseOnDisconnect(true);

  auto service = _server->createService(BLEUUID(SERVICE_UUID));

  _characteristic = service->createCharacteristic(
      BLEUUID(CHARACTERISTIC_UUID),
      NIMBLE_PROPERTY::READ |
          NIMBLE_PROPERTY::WRITE |
          NIMBLE_PROPERTY::NOTIFY |
          NIMBLE_PROPERTY::WRITE_NR);

  _characteristic->setCallbacks(new MyCharacteristicCallbacks(this));

  auto _security = new NimBLESecurity();
  _security->setAuthenticationMode(ESP_LE_AUTH_BOND);

  service->start();

  _advertising = _server->getAdvertising();
  _advertising->addServiceUUID(service->getUUID());
  _advertising->enableScanResponse(true);
  _advertising->setName(deviceName);
  _advertising->setAppearance(0x00);
  _advertising->start();

  return true;
}

#define BLEMIDI_CREATE_INSTANCE(DeviceName, Name)                             \
  BLEMIDI_NAMESPACE::BLEMIDI_Transport<                                      \
      BLEMIDI_NAMESPACE::BLEMIDI_ESP32_NimBLE> BLE##Name(DeviceName);        \
  MIDI_NAMESPACE::MidiInterface<                                             \
      BLEMIDI_NAMESPACE::BLEMIDI_Transport<                                  \
          BLEMIDI_NAMESPACE::BLEMIDI_ESP32_NimBLE>,                          \
      BLEMIDI_NAMESPACE::MySettings>                                         \
      Name((BLEMIDI_NAMESPACE::BLEMIDI_Transport<                            \
          BLEMIDI_NAMESPACE::BLEMIDI_ESP32_NimBLE>&)BLE##Name);

#define BLEMIDI_CREATE_DEFAULT_INSTANCE() \
  BLEMIDI_CREATE_INSTANCE("Esp32-NimBLE-MIDI", MIDI)

END_BLEMIDI_NAMESPACE
