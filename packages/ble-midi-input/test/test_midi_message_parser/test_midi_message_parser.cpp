#include <unity.h>

#include <array>

#include "MidiMessageParser.h"

class CapturingMessageSink : public MidiMessageSink {
public:
  void onMidiMessage(const MidiMessage& message) override {
    messages[count] = message;
    count += 1;
  }

  void onUnsupportedStatusByte(uint8_t statusByte) override {
    unsupportedStatusBytes[unsupportedCount] = statusByte;
    unsupportedCount += 1;
  }

  std::array<MidiMessage, 8> messages{};
  size_t count = 0;
  std::array<uint8_t, 8> unsupportedStatusBytes{};
  size_t unsupportedCount = 0;
};

void test_midi_parser_decodes_note_on_and_running_status() {
  CapturingMessageSink sink;
  MidiMessageParser parser;

  TEST_ASSERT_TRUE(parser.parseByte(0x90, sink));
  TEST_ASSERT_TRUE(parser.parseByte(60, sink));
  TEST_ASSERT_TRUE(parser.parseByte(100, sink));
  TEST_ASSERT_TRUE(parser.parseByte(64, sink));
  TEST_ASSERT_TRUE(parser.parseByte(80, sink));

  TEST_ASSERT_EQUAL_UINT32(2, sink.count);
  TEST_ASSERT_EQUAL(MidiMessageType::NoteOn, sink.messages[1].type);
  TEST_ASSERT_EQUAL_UINT8(64, sink.messages[1].data1);
  TEST_ASSERT_EQUAL_UINT8(80, sink.messages[1].data2);
}

void test_midi_parser_decodes_control_change_and_pitch_bend() {
  CapturingMessageSink sink;
  MidiMessageParser parser;

  TEST_ASSERT_TRUE(parser.parseByte(0xb2, sink));
  TEST_ASSERT_TRUE(parser.parseByte(64, sink));
  TEST_ASSERT_TRUE(parser.parseByte(127, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0xe2, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0, sink));
  TEST_ASSERT_TRUE(parser.parseByte(64, sink));

  TEST_ASSERT_EQUAL_UINT32(2, sink.count);
  TEST_ASSERT_EQUAL(MidiMessageType::ControlChange, sink.messages[0].type);
  TEST_ASSERT_EQUAL(MidiMessageType::PitchBend, sink.messages[1].type);
  TEST_ASSERT_EQUAL_INT(0, sink.messages[1].bendValue);
}

void test_midi_parser_decodes_note_off_and_pitch_bend_limits() {
  CapturingMessageSink sink;
  MidiMessageParser parser;

  TEST_ASSERT_TRUE(parser.parseByte(0x82, sink));
  TEST_ASSERT_TRUE(parser.parseByte(60, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0xe2, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0, sink));
  TEST_ASSERT_TRUE(parser.parseByte(127, sink));
  TEST_ASSERT_TRUE(parser.parseByte(127, sink));

  TEST_ASSERT_EQUAL_UINT32(3, sink.count);
  TEST_ASSERT_EQUAL(MidiMessageType::NoteOff, sink.messages[0].type);
  TEST_ASSERT_EQUAL_UINT8(2, sink.messages[0].channel);
  TEST_ASSERT_EQUAL_INT(-8192, sink.messages[1].bendValue);
  TEST_ASSERT_EQUAL_INT(8191, sink.messages[2].bendValue);
}

void test_midi_parser_rejects_unsupported_and_incomplete_messages() {
  CapturingMessageSink sink;
  MidiMessageParser parser;

  TEST_ASSERT_FALSE(parser.parseByte(0xc0, sink));
  TEST_ASSERT_FALSE(parser.parseByte(60, sink));
  TEST_ASSERT_TRUE(parser.parseByte(0x90, sink));
  TEST_ASSERT_TRUE(parser.parseByte(60, sink));
  TEST_ASSERT_FALSE(parser.parseByte(0xc0, sink));
  TEST_ASSERT_EQUAL_UINT32(0, sink.count);
  TEST_ASSERT_EQUAL_UINT32(2, sink.unsupportedCount);
  TEST_ASSERT_EQUAL_UINT8(0xc0, sink.unsupportedStatusBytes[0]);
  TEST_ASSERT_EQUAL_UINT8(0xc0, sink.unsupportedStatusBytes[1]);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_midi_parser_decodes_note_on_and_running_status);
  RUN_TEST(test_midi_parser_decodes_control_change_and_pitch_bend);
  RUN_TEST(test_midi_parser_decodes_note_off_and_pitch_bend_limits);
  RUN_TEST(test_midi_parser_rejects_unsupported_and_incomplete_messages);
  return UNITY_END();
}
