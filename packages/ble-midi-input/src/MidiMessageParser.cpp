#include "MidiMessageParser.h"

namespace {
uint8_t dataCountForStatus(uint8_t status) {
  switch (status >> 4) {
    case 0x8:
    case 0x9:
    case 0xb:
    case 0xe:
      return 2;
    default:
      return 0;
  }
}
}

bool MidiMessageParser::parseByte(uint8_t byte, MidiMessageSink& sink) {
  if ((byte & 0x80) != 0) {
    const uint8_t expectedDataCount = dataCountForStatus(byte);
    if (expectedDataCount == 0) {
      sink.onUnsupportedStatusByte(byte);
      runningStatus_ = 0;
      dataCount_ = 0;
      expectedDataCount_ = 0;
      messageInProgress_ = false;
      return false;
    }

    runningStatus_ = byte;
    dataCount_ = 0;
    expectedDataCount_ = expectedDataCount;
    messageInProgress_ = true;
    return true;
  }

  if (runningStatus_ == 0 || expectedDataCount_ == 0) {
    return false;
  }

  data_[dataCount_] = byte;
  dataCount_ += 1;
  if (dataCount_ < expectedDataCount_) {
    return true;
  }

  const uint8_t command = runningStatus_ >> 4;
  const uint8_t channel = runningStatus_ & 0x0f;
  MidiMessage message = {
      MidiMessageType::NoteOn,
      channel,
      data_[0],
      data_[1],
      0,
  };

  switch (command) {
    case 0x8:
      message.type = MidiMessageType::NoteOff;
      break;
    case 0x9:
      message.type = MidiMessageType::NoteOn;
      break;
    case 0xb:
      message.type = MidiMessageType::ControlChange;
      break;
    case 0xe:
      message.type = MidiMessageType::PitchBend;
      message.data1 = 0;
      message.data2 = 0;
      message.bendValue =
          (static_cast<int>(data_[1]) << 7) + data_[0] - 8192;
      break;
    default:
      return false;
  }

  dataCount_ = 0;
  messageInProgress_ = false;
  sink.onMidiMessage(message);
  return true;
}

bool MidiMessageParser::hasPendingMessage() const {
  return messageInProgress_;
}
