#include "BleMidiPacketParser.h"

namespace {
int normalizePitchBend(uint16_t bendValue) {
  return static_cast<int>(bendValue) - 8192;
}
}

bool BleMidiPacketParser::parse(
    const uint8_t* data,
    size_t size,
    BleMidiMessageSink& sink) {
  if (data == nullptr || size < 3 || (data[0] & 0x80) == 0 ||
      (data[1] & 0x80) == 0) {
    return false;
  }

  const uint8_t* cursor = data + 1;
  const uint8_t* end = data + size;
  uint8_t runningStatus = 0;

  while (cursor < end) {
    if ((*cursor & 0x80) != 0) {
      cursor += 1;
    }

    if (cursor >= end) {
      return false;
    }

    if ((*cursor & 0x80) != 0) {
      runningStatus = *cursor;
      cursor += 1;
    }

    if (runningStatus == 0 ||
        !parseMidiMessage(runningStatus, cursor, end, sink)) {
      return false;
    }
  }

  return true;
}

bool BleMidiPacketParser::parseMidiMessage(
    uint8_t status,
    const uint8_t*& cursor,
    const uint8_t* end,
    BleMidiMessageSink& sink) {
  const uint8_t command = status >> 4;
  const uint8_t channel = status & 0x0f;

  switch (command) {
    case 0x8:
    case 0x9: {
      if (end - cursor < 2) return false;
      const BleMidiMessage message = {
          command == 0x9 ? BleMidiMessageType::NoteOn
                         : BleMidiMessageType::NoteOff,
          channel, cursor[0], cursor[1], 0};
      cursor += 2;
      sink.onBleMidiMessage(message);
      return true;
    }
    case 0xb: {
      if (end - cursor < 2) return false;
      const BleMidiMessage message = {
          BleMidiMessageType::ControlChange,
          channel, cursor[0], cursor[1], 0};
      cursor += 2;
      sink.onBleMidiMessage(message);
      return true;
    }
    case 0xe: {
      if (end - cursor < 2) return false;
      const uint16_t bendValue =
          (static_cast<uint16_t>(cursor[1] & 0x7f) << 7) |
          static_cast<uint16_t>(cursor[0] & 0x7f);
      const BleMidiMessage message = {
          BleMidiMessageType::PitchBend,
          channel, 0, 0, normalizePitchBend(bendValue)};
      cursor += 2;
      sink.onBleMidiMessage(message);
      return true;
    }
    default:
      return false;
  }
}
