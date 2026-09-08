#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

constexpr size_t MIDI_NOTE_NAME_TEXT_LENGTH = 5;

using MidiNoteName = std::array<char, MIDI_NOTE_NAME_TEXT_LENGTH>;

// Return a short note name using the common MIDI octave convention where
// middle C is C4, so MIDI note 60 maps to "C4".
MidiNoteName midiNoteName(uint8_t note);
