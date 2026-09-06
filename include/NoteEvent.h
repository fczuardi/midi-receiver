#pragma once

#include <cstdint>

enum class NoteEventType : uint8_t {
  NoteOn,
  NoteOff,
};

// Typed musical note event produced by MIDI input code.
//
// This contract intentionally contains no BLE, parser, display, or hardware
// output details. Pitch bend and control change should use separate event types.
//
// Producers are responsible for validating or normalizing raw MIDI data before
// creating this event:
// - channel uses the human-facing MIDI range 1-16;
// - note uses the MIDI note range 0-127;
// - velocity uses the MIDI velocity range 0-127.
struct NoteEvent {
  NoteEventType type;
  uint8_t channel;
  uint8_t note;
  uint8_t velocity;
};

// Build a NoteEvent from parsed MIDI note fields.
// MIDI convention treats Note On with velocity 0 as Note Off.
NoteEvent makeNoteEvent(
    NoteEventType type,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity);
