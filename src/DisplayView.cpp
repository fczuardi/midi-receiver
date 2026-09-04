#include "DisplayView.h"

#include <M5Unified.h>

namespace {
constexpr int32_t TITLE_ROW_Y = 0;
constexpr int32_t BLE_ROW_Y = 27;
constexpr int32_t UPTIME_ROW_Y = 43;
constexpr int32_t MIDI_PACKETS_ROW_Y = 59;
constexpr int32_t MIDI_BYTES_ROW_Y = 75;

constexpr int32_t LABEL_COLUMN_X = 0;
constexpr int32_t VALUE_COLUMN_X = 96;
constexpr int32_t VALUE_COLUMN_WIDTH = 120;
constexpr int32_t VALUE_ROW_HEIGHT = 12;
}

void DisplayView::begin() {
  M5.Display.setRotation(1);
  M5.Display.setBrightness(96);
}

void DisplayView::update(const AppState& appState) {
  if (appState.fullDisplayRefreshNeeded()) {
    drawStaticLayout(appState);
    drawUptimeValue(appState);
    drawMidiActivityValues(appState);
    return;
  }

  if (appState.uptimeRefreshNeeded()) {
    drawUptimeValue(appState);
  }

  if (appState.midiActivityRefreshNeeded()) {
    drawMidiActivityValues(appState);
  }
}

void DisplayView::drawStaticLayout(const AppState& appState) {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setCursor(LABEL_COLUMN_X, TITLE_ROW_Y);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(2);
  M5.Display.println("BLE MIDI RX");

  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.setCursor(LABEL_COLUMN_X, BLE_ROW_Y);
  M5.Display.print("BLE: ");
  M5.Display.println(bleStateLabel(appState.bleConnectionState()));

  M5.Display.setCursor(LABEL_COLUMN_X, UPTIME_ROW_Y);
  M5.Display.print("Uptime: ");

  M5.Display.setCursor(LABEL_COLUMN_X, MIDI_PACKETS_ROW_Y);
  M5.Display.print("MIDI packets: ");

  M5.Display.setCursor(LABEL_COLUMN_X, MIDI_BYTES_ROW_Y);
  M5.Display.print("Last bytes: ");
}

void DisplayView::drawUptimeValue(const AppState& appState) {
  // Only clear the rectangle occupied by the changing number. This avoids the
  // visible full-screen blink caused by fillScreen() on every uptime tick.
  M5.Display.fillRect(
      VALUE_COLUMN_X,
      UPTIME_ROW_Y,
      VALUE_COLUMN_WIDTH,
      VALUE_ROW_HEIGHT,
      TFT_BLACK);
  M5.Display.setCursor(VALUE_COLUMN_X, UPTIME_ROW_Y);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.printf("%lu s", appState.uptimeSeconds());
}

void DisplayView::drawMidiActivityValues(const AppState& appState) {
  // MIDI activity can update faster than uptime. Keep these updates localized
  // so incoming packets do not make the whole screen flash.
  M5.Display.fillRect(
      VALUE_COLUMN_X,
      MIDI_PACKETS_ROW_Y,
      VALUE_COLUMN_WIDTH,
      VALUE_ROW_HEIGHT,
      TFT_BLACK);
  M5.Display.setCursor(VALUE_COLUMN_X, MIDI_PACKETS_ROW_Y);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.print(appState.receivedMidiPacketCount());

  M5.Display.fillRect(
      VALUE_COLUMN_X,
      MIDI_BYTES_ROW_Y,
      VALUE_COLUMN_WIDTH,
      VALUE_ROW_HEIGHT,
      TFT_BLACK);
  M5.Display.setCursor(VALUE_COLUMN_X, MIDI_BYTES_ROW_Y);
  M5.Display.print(appState.lastMidiPacketSize());
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
