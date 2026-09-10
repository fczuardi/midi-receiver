#include "BleMidiPacketDecoder.h"

bool BleMidiPacketDecoder::parse(
    const uint8_t* data,
    size_t size,
    MidiMessageSink& sink) {
  if (data == nullptr || size < 3 || (data[0] & 0x80) == 0 ||
      (data[1] & 0x80) == 0) {
    return false;
  }

  MidiMessageParser midiParser;
  const uint8_t* cursor = data + 1;
  const uint8_t* end = data + size;

  while (cursor < end) {
    if ((*cursor & 0x80) != 0) {
      cursor += 1;
    }

    if (cursor >= end) {
      return false;
    }

    if (!midiParser.parseByte(*cursor, sink)) {
      return false;
    }
    cursor += 1;
  }

  return !midiParser.hasPendingMessage();
}
