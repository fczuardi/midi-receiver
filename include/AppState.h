#pragma once

#include <Arduino.h>

enum class BleConnectionState {
  Starting,
  Advertising,
  Connected,
};

// AppState is the small shared model for the application.
// Other modules read or update this state instead of each owning their own
// copy of the current BLE/display facts.
class AppState {
public:
  // Return the current high-level BLE state shown on the display.
  BleConnectionState bleConnectionState() const;

  // Return the number of seconds since the firmware booted.
  uint32_t uptimeSeconds() const;

  // Tell the display layer whether anything changed since the last redraw.
  bool displayRefreshNeeded() const;

  // Update BLE state and request a display redraw when the value changes.
  void setBleConnectionState(BleConnectionState state);

  // Update uptime and request a display redraw when the value changes.
  void setUptimeSeconds(uint32_t uptimeSeconds);

  // Clear the redraw flag after the display has rendered the latest state.
  void markDisplayRefreshed();

private:
  BleConnectionState bleConnectionState_ = BleConnectionState::Starting;
  uint32_t uptimeSeconds_ = 0;
  bool displayRefreshNeeded_ = true;
};
