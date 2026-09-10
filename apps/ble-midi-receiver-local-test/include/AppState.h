#pragma once

#include <cstddef>
#include <cstdint>

#include "ActiveNotes.h"
#include "NoteEvent.h"

enum class BleConnectionState {
  Starting,
  Advertising,
  Connected,
};

enum class MidiActivityKind {
  None,
  NoteOn,
  NoteOff,
  ControlChange,
  PitchBend,
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

  // Return how many Note On events were ignored because the active-note table
  // was already full.
  uint32_t droppedActiveNoteCount() const;

  // Return the kind of the most recent parsed MIDI activity.
  MidiActivityKind lastMidiActivityKind() const;

  // Return parsed fields from the most recent Note On/Off message.
  uint8_t lastMidiChannel() const;
  uint8_t lastMidiNote() const;
  uint8_t lastMidiVelocity() const;

  // Return parsed fields from the most recent Control Change message.
  uint8_t lastControlChangeChannel() const;
  uint8_t lastControlChangeNumber() const;
  uint8_t lastControlChangeValue() const;
  bool hasControlChange() const;

  // Return the current sustain pedal/hold state observed from CC 64.
  bool sustainEnabled() const;

  // Return parsed fields from the most recent Pitch Bend message.
  uint8_t lastPitchBendChannel() const;
  int lastPitchBendValue() const;
  bool hasPitchBend() const;

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
  // A Note On with velocity 0 is normalized to Note Off here as a defensive
  // MIDI convention, even though the MIDI library is expected to do this too.
  void recordMidiActivity(
      MidiActivityKind kind,
      uint8_t channel,
      uint8_t note,
      uint8_t velocity,
      uint32_t activityAtMs);

  // Record a typed NoteEvent produced by the MIDI input boundary.
  void recordNoteEvent(const NoteEvent& event, uint32_t activityAtMs);

  // Record a Control Change message without applying musical interpretation,
  // except for exposing CC 64 as the observed sustain/hold state.
  void recordControlChange(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue,
      uint32_t activityAtMs);

  // Record a Pitch Bend message as a centered signed value from the MIDI parser.
  void recordPitchBend(uint8_t channel, int bendValue, uint32_t activityAtMs);

  // Clear display refresh flags after the display has rendered the latest state.
  void markDisplayRefreshed();

private:
  BleConnectionState bleConnectionState_ = BleConnectionState::Starting;
  uint32_t uptimeSeconds_ = 0;
  uint32_t receivedMidiMessageCount_ = 0;
  uint32_t droppedActiveNoteCount_ = 0;
  MidiActivityKind lastMidiActivityKind_ = MidiActivityKind::None;
  uint8_t lastMidiChannel_ = 0;
  uint8_t lastMidiNote_ = 0;
  uint8_t lastMidiVelocity_ = 0;
  uint8_t lastControlChangeChannel_ = 0;
  uint8_t lastControlChangeNumber_ = 0;
  uint8_t lastControlChangeValue_ = 0;
  bool hasControlChange_ = false;
  bool sustainEnabled_ = false;
  uint8_t lastPitchBendChannel_ = 0;
  int lastPitchBendValue_ = 0;
  bool hasPitchBend_ = false;
  uint32_t lastMidiActivityAtMs_ = 0;
  ActiveNotes activeNotes_;
  bool fullDisplayRefreshNeeded_ = true;
  bool uptimeRefreshNeeded_ = true;
  bool midiActivityRefreshNeeded_ = true;
};
