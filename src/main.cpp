#include <Arduino.h>
#include <M5Unified.h>

namespace {
constexpr uint32_t SCREEN_REFRESH_MS = 1000;

uint32_t lastScreenRefreshMs = 0;

void drawStatusScreen() {
  const uint32_t uptimeSeconds = millis() / 1000;

  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("BLE MIDI RX");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.println("Baseline firmware");
  M5.Display.println("Status: booted");
  M5.Display.printf("Uptime: %lu s\n", uptimeSeconds);
  M5.Display.println();
  M5.Display.println("Next: BLE MIDI");
}
}

void setup() {
  auto config = M5.config();
  M5.begin(config);

  Serial.begin(115200);
  delay(200);

  M5.Display.setRotation(1);
  M5.Display.setBrightness(96);

  Serial.println();
  Serial.println("M5StickC Plus2 BLE MIDI receiver");
  Serial.println("Baseline firmware booted");

  drawStatusScreen();
}

void loop() {
  M5.update();

  const uint32_t nowMs = millis();
  if (nowMs - lastScreenRefreshMs >= SCREEN_REFRESH_MS) {
    lastScreenRefreshMs = nowMs;
    drawStatusScreen();
    Serial.printf("uptime=%lu\n", nowMs / 1000);
  }
}
