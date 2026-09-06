#include "NoteEvent.h"

NoteEvent makeNoteEvent(
    NoteEventType type,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity) {
  if (type == NoteEventType::NoteOn && velocity == 0) {
    return {NoteEventType::NoteOff, channel, note, velocity};
  }

  return {type, channel, note, velocity};
}
