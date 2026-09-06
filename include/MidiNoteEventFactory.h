#pragma once

#include <cstdint>

#include "NoteEvent.h"

// Build a shared NoteEvent from parsed MIDI note fields.
//
// Normalization stays in the receiver boundary because raw MIDI conventions are
// transport/parser concerns, not instrument policy.
NoteEvent makeNoteEvent(
    NoteEventType type,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity);
