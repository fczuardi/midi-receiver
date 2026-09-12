#include "MidiMessageParser.h"

namespace {
uint8_t dataCountForStatus(uint8_t status) {
  switch (status >> 4) {
    case 0x8:
    case 0x9:
    case 0xa:
    case 0xb:
    case 0xe:
      return 2;
    case 0xc:
    case 0xd:
      return 1;
    default:
      return 0;
  }
}

bool isSupportedStatus(uint8_t status) {
  return (status >> 4) == 0x8 || (status >> 4) == 0x9 ||
         (status >> 4) == 0xb || (status >> 4) == 0xe;
}
}

bool MidiMessageParser::parseByte(uint8_t byte, MidiMessageSink& sink) {
  if ((byte & 0x80) != 0) {
    const uint8_t expectedDataCount = dataCountForStatus(byte);
    if (expectedDataCount == 0) {
      UnsupportedMidiMessage message;
      message.size = 1;
      message.bytes[0] = byte;
      sink.onUnsupportedMessage(message);
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
    unsupportedMessage_ = !isSupportedStatus(byte);
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

  if (unsupportedMessage_) {
    UnsupportedMidiMessage message;
    message.size = expectedDataCount_ + 1;
    message.bytes[0] = runningStatus_;
    message.bytes[1] = data_[0];
    if (expectedDataCount_ > 1) {
      message.bytes[2] = data_[1];
    }
    dataCount_ = 0;
    messageInProgress_ = false;
    sink.onUnsupportedMessage(message);
    return true;
  }

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
  unsupportedMessage_ = false;
  sink.onMidiMessage(message);
  return true;
}

bool MidiMessageParser::hasPendingMessage() const {
  return messageInProgress_;
}
