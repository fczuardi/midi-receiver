#pragma once

#include <cstddef>
#include <cstdint>

enum class BleMidiMessageType : uint8_t {
  NoteOn,
  NoteOff,
  ControlChange,
  PitchBend,
};

struct BleMidiMessage {
  BleMidiMessageType type;
  uint8_t channel;
  uint8_t data1;
  uint8_t data2;
  int bendValue;
};

class BleMidiMessageSink {
public:
  virtual ~BleMidiMessageSink() = default;
  virtual void onBleMidiMessage(const BleMidiMessage& message) = 0;
};

// Parses the supported BLE MIDI packet messages without Arduino or BLE APIs.
class BleMidiPacketParser {
public:
  // Returns false for malformed or unsupported data. Messages parsed before
  // an invalid message are still delivered to the sink.
  static bool parse(const uint8_t* data, size_t size, BleMidiMessageSink& sink);

private:
  static bool parseMidiMessage(
      uint8_t status,
      const uint8_t*& cursor,
      const uint8_t* end,
      BleMidiMessageSink& sink);
};
