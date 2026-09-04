#include "DisplayView.h"

#include <M5Unified.h>

void DisplayView::begin() {
  M5.Display.setRotation(1);
  M5.Display.setBrightness(96);
}

void DisplayView::update(const AppState& appState) {
  if (!appState.displayRefreshNeeded()) {
    return;
  }

  draw(appState);
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

void DisplayView::draw(const AppState& appState) {
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
  M5.Display.printf("Uptime: %lu s\n", appState.uptimeSeconds());
  M5.Display.println();
  M5.Display.println("Waiting for client");
}

