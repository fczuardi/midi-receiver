#pragma once

#include <cstddef>
#include <cstdint>

#include "MidiMessageParser.h"

// Removes BLE MIDI packet framing and delegates MIDI bytes to the generic
// message parser. This is an internal transport detail, not a public package
// abstraction.
class BleMidiPacketDecoder {
public:
  static bool parse(
      const uint8_t* data,
      size_t size,
      MidiMessageSink& sink);
};
