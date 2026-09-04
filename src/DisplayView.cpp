#include "DisplayView.h"

#include <M5Unified.h>

namespace {
constexpr int32_t UPTIME_VALUE_X = 54;
constexpr int32_t UPTIME_VALUE_Y = 39;
constexpr int32_t UPTIME_VALUE_WIDTH = 80;
constexpr int32_t UPTIME_VALUE_HEIGHT = 12;
}

void DisplayView::begin() {
  M5.Display.setRotation(1);
  M5.Display.setBrightness(96);
}

void DisplayView::update(const AppState& appState) {
  if (appState.fullDisplayRefreshNeeded()) {
    drawStaticLayout(appState);
    drawUptimeValue(appState);
    return;
  }

  if (appState.uptimeRefreshNeeded()) {
    drawUptimeValue(appState);
  }
}

void DisplayView::drawStaticLayout(const AppState& appState) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(0, 0);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("BLE MIDI RX");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println();
  M5.Display.print("BLE: ");
  M5.Display.println(bleStateLabel(appState.bleConnectionState()));
  M5.Display.print("Uptime: ");
  M5.Display.println();
  M5.Display.println();
  M5.Display.println("Waiting for client");
}

void DisplayView::drawUptimeValue(const AppState& appState) {
  // Only clear the rectangle occupied by the changing number. This avoids the
  // visible full-screen blink caused by fillScreen() on every uptime tick.
  M5.Display.fillRect(
      UPTIME_VALUE_X,
      UPTIME_VALUE_Y,
      UPTIME_VALUE_WIDTH,
      UPTIME_VALUE_HEIGHT,
      TFT_BLACK);
  M5.Display.setCursor(UPTIME_VALUE_X, UPTIME_VALUE_Y);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.printf("%lu s", appState.uptimeSeconds());
}

const char* DisplayView::bleStateLabel(BleConnectionState state) const {
  switch (state) {
    case BleConnectionState::Starting:
      return "starting";
    case BleConnectionState::Advertising:
      return "advertising";
    case BleConnectionState::Connected:
      return "connected";
  }

  return "unknown";
}
