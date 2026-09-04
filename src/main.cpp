#include <Arduino.h>
#include <M5Unified.h>

#include "AppState.h"
#include "BleMidiPeripheral.h"
#include "DisplayView.h"

namespace {
constexpr uint32_t UPTIME_REFRESH_INTERVAL_MS = 1000;

AppState appState;
DisplayView displayView;
BleMidiPeripheral bleMidiPeripheral(appState);
uint32_t lastUptimeRefreshMs = 0;
}

void setup() {
  auto config = M5.config();
  M5.begin(config);

  Serial.begin(115200);
  delay(200);

  displayView.begin();

  Serial.println();
  Serial.println("M5StickC Plus2 BLE MIDI receiver");
  Serial.println("Firmware booted");

  bleMidiPeripheral.begin();
  displayView.update(appState);
}

void loop() {
  M5.update();
  bleMidiPeripheral.update();

  const uint32_t nowMs = millis();
  if (nowMs - lastUptimeRefreshMs >= UPTIME_REFRESH_INTERVAL_MS) {
    lastUptimeRefreshMs = nowMs;
    appState.setUptimeSeconds(nowMs / 1000);
    Serial.printf("uptime=%lu\n", nowMs / 1000);
  }

  displayView.update(appState);
  appState.markDisplayRefreshed();
}
