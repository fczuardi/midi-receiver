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

  // Return how many BLE MIDI packets have arrived since boot.
  uint32_t receivedMidiPacketCount() const;

  // Return the byte length of the most recent BLE MIDI packet.
  size_t lastMidiPacketSize() const;

  // Return millis() timestamp of the most recent BLE MIDI packet.
  uint32_t lastMidiPacketAtMs() const;

  // Tell the display layer whether the static layout must be redrawn.
  bool fullDisplayRefreshNeeded() const;

  // Tell the display layer whether only the uptime value changed.
  bool uptimeRefreshNeeded() const;

  // Tell the display layer whether only the MIDI activity values changed.
  bool midiActivityRefreshNeeded() const;

  // Update BLE state and request a display redraw when the value changes.
  void setBleConnectionState(BleConnectionState state);

  // Update uptime and request a display redraw when the value changes.
  void setUptimeSeconds(uint32_t uptimeSeconds);

  // Record raw BLE MIDI packet activity without parsing the packet contents.
  void recordMidiPacketActivity(size_t packetSize, uint32_t packetReceivedAtMs);

  // Clear display refresh flags after the display has rendered the latest state.
  void markDisplayRefreshed();

private:
  BleConnectionState bleConnectionState_ = BleConnectionState::Starting;
  uint32_t uptimeSeconds_ = 0;
  uint32_t receivedMidiPacketCount_ = 0;
  size_t lastMidiPacketSize_ = 0;
  uint32_t lastMidiPacketAtMs_ = 0;
  bool fullDisplayRefreshNeeded_ = true;
  bool uptimeRefreshNeeded_ = true;
  bool midiActivityRefreshNeeded_ = true;
};
