#pragma once

#include <cstdint>

enum class MidiMessageType : uint8_t {
  NoteOn,
  NoteOff,
  ControlChange,
  PitchBend,
};

struct MidiMessage {
  MidiMessageType type;
  uint8_t channel;
  uint8_t data1;
  uint8_t data2;
  int bendValue;
};

struct UnsupportedMidiMessage {
  uint8_t size = 0;
  uint8_t bytes[3] = {};
};

class MidiMessageSink {
public:
  virtual ~MidiMessageSink() = default;
  virtual void onMidiMessage(const MidiMessage& message) = 0;

  // Diagnostic hook for status bytes outside the parser's supported subset.
  // The default keeps existing parser sinks focused on decoded messages.
  virtual void onUnsupportedMessage(const UnsupportedMidiMessage&) {
  }
};

// Parses the supported MIDI channel messages from a byte stream.
class MidiMessageParser {
public:
  // Returns false for unsupported status bytes or invalid data bytes.
  // Incomplete messages remain pending until more bytes arrive.
  bool parseByte(uint8_t byte, MidiMessageSink& sink);

  bool hasPendingMessage() const;

private:
  uint8_t runningStatus_ = 0;
  uint8_t data_[2] = {};
  uint8_t dataCount_ = 0;
  uint8_t expectedDataCount_ = 0;
  bool messageInProgress_ = false;
  bool unsupportedMessage_ = false;
};
