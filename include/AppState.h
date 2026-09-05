#pragma once

#include <Arduino.h>

#include "ActiveNotes.h"

enum class BleConnectionState {
  Starting,
  Advertising,
  Connected,
};

enum class MidiActivityKind {
  None,
  ActiveSensing,
  NoteOn,
  NoteOff,
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

  // Return how many parsed MIDI messages have arrived since boot.
  uint32_t receivedMidiMessageCount() const;

  // Return how many Active Sensing messages have arrived since boot.
  uint32_t activeSensingMessageCount() const;

  // Return the kind of the most recent parsed MIDI activity.
  MidiActivityKind lastMidiActivityKind() const;

  // Return parsed fields from the most recent Note On/Off message.
  uint8_t lastMidiChannel() const;
  uint8_t lastMidiNote() const;
  uint8_t lastMidiVelocity() const;

  // Return millis() timestamp of the most recent MIDI activity.
  uint32_t lastMidiActivityAtMs() const;

  // Return currently held notes across all channels.
  size_t activeNoteCount() const;
  ActiveNote activeNoteAt(size_t index) const;

  // Tell the display layer whether the static layout must be redrawn.
  bool fullDisplayRefreshNeeded() const;

  // Tell the display layer whether only the uptime value changed.
  bool uptimeRefreshNeeded() const;

  // Tell the display layer whether only the MIDI activity values changed.
  bool midiActivityRefreshNeeded() const;

  // Update BLE state and request a display redraw when the value changes.
  void setBleConnectionState(BleConnectionState state);

  // Clear active notes because the sender disconnected before releasing them.
  void clearActiveNotes();

  // Update uptime and request a display redraw when the value changes.
  void setUptimeSeconds(uint32_t uptimeSeconds);

  // Record a parsed MIDI activity event produced by the MIDI library.
  void recordMidiActivity(
      MidiActivityKind kind,
      uint8_t channel,
      uint8_t note,
      uint8_t velocity,
      uint32_t activityAtMs);

  // Clear display refresh flags after the display has rendered the latest state.
  void markDisplayRefreshed();

private:
  BleConnectionState bleConnectionState_ = BleConnectionState::Starting;
  uint32_t uptimeSeconds_ = 0;
  uint32_t receivedMidiMessageCount_ = 0;
  uint32_t activeSensingMessageCount_ = 0;
  MidiActivityKind lastMidiActivityKind_ = MidiActivityKind::None;
  uint8_t lastMidiChannel_ = 0;
  uint8_t lastMidiNote_ = 0;
  uint8_t lastMidiVelocity_ = 0;
  uint32_t lastMidiActivityAtMs_ = 0;
  ActiveNotes activeNotes_;
  bool fullDisplayRefreshNeeded_ = true;
  bool uptimeRefreshNeeded_ = true;
  bool midiActivityRefreshNeeded_ = true;
};
