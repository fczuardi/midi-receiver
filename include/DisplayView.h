#pragma once

#include "AppState.h"

// Owns all drawing on the built-in M5StickC Plus2 display.
//
// Keeping display code out of main.cpp makes it easier to change the layout
// without touching BLE or application state logic.
class DisplayView {
public:
  // Configure screen rotation, brightness, and other display-only settings.
  void begin();

  // Redraw the screen only when AppState says something visible changed.
  void update(const AppState& appState);

private:
  const char* bleStateLabel(BleConnectionState state) const;
  void draw(const AppState& appState);
};
