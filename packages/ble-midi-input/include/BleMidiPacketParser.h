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

// Parses the BLE MIDI packet payload defined by the BLE MIDI specification.
// The parser owns no state between packets and has no Arduino or BLE dependency.
class BleMidiPacketParser {
public:
  // Returns false when the packet contains invalid or unsupported MIDI data.
  // Messages parsed before an invalid message are still delivered.
  static bool parse(
      const uint8_t* data,
      size_t size,
      BleMidiMessageSink& sink);

private:
  static bool parseMidiMessage(
      uint8_t status,
      const uint8_t*& cursor,
      const uint8_t* end,
      BleMidiMessageSink& sink);
};
