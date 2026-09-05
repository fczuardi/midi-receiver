#include "MidiNoteName.h"

#include <cstdio>

namespace {
constexpr const char* NOTE_NAMES[] = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B",
};

constexpr uint8_t NOTES_PER_OCTAVE = 12;
constexpr int8_t MIDI_OCTAVE_OFFSET = -1;
}

MidiNoteName midiNoteName(uint8_t note) {
  MidiNoteName result{};
  const char* noteName = NOTE_NAMES[note % NOTES_PER_OCTAVE];
  const int octave = static_cast<int>(note / NOTES_PER_OCTAVE) + MIDI_OCTAVE_OFFSET;

  std::snprintf(result.data(), result.size(), "%s%d", noteName, octave);
  return result;
}
