#include <unity.h>

#include <array>

#include "BleMidiPacketParser.h"

class CapturingMessageSink : public BleMidiMessageSink {
public:
  void onBleMidiMessage(const BleMidiMessage& message) override {
    messages[count] = message;
    count += 1;
  }

  std::array<BleMidiMessage, 8> messages{};
  size_t count = 0;
};

void test_parser_decodes_note_on_with_timestamp() {
  const uint8_t packet[] = {0x80, 0x81, 0x90, 60, 100};
  CapturingMessageSink sink;

  TEST_ASSERT_TRUE(BleMidiPacketParser::parse(packet, sizeof(packet), sink));
  TEST_ASSERT_EQUAL_UINT32(1, sink.count);
  TEST_ASSERT_EQUAL(BleMidiMessageType::NoteOn, sink.messages[0].type);
  TEST_ASSERT_EQUAL_UINT8(0, sink.messages[0].channel);
  TEST_ASSERT_EQUAL_UINT8(60, sink.messages[0].data1);
  TEST_ASSERT_EQUAL_UINT8(100, sink.messages[0].data2);
}

void test_parser_decodes_running_status_messages() {
  const uint8_t packet[] = {
      0x80, 0x81, 0x90, 60, 100, 0x82, 64, 80};
  CapturingMessageSink sink;

  TEST_ASSERT_TRUE(BleMidiPacketParser::parse(packet, sizeof(packet), sink));
  TEST_ASSERT_EQUAL_UINT32(2, sink.count);
  TEST_ASSERT_EQUAL(BleMidiMessageType::NoteOn, sink.messages[1].type);
  TEST_ASSERT_EQUAL_UINT8(64, sink.messages[1].data1);
  TEST_ASSERT_EQUAL_UINT8(80, sink.messages[1].data2);
}

void test_parser_decodes_control_change_and_pitch_bend() {
  const uint8_t packet[] = {
      0x80, 0x81, 0xb2, 64, 127, 0x82, 0xe2, 0x00, 0x40};
  CapturingMessageSink sink;

  TEST_ASSERT_TRUE(BleMidiPacketParser::parse(packet, sizeof(packet), sink));
  TEST_ASSERT_EQUAL_UINT32(2, sink.count);
  TEST_ASSERT_EQUAL(BleMidiMessageType::ControlChange, sink.messages[0].type);
  TEST_ASSERT_EQUAL_UINT8(2, sink.messages[0].channel);
  TEST_ASSERT_EQUAL_UINT8(64, sink.messages[0].data1);
  TEST_ASSERT_EQUAL_UINT8(127, sink.messages[0].data2);
  TEST_ASSERT_EQUAL(BleMidiMessageType::PitchBend, sink.messages[1].type);
  TEST_ASSERT_EQUAL_INT(0, sink.messages[1].bendValue);
}

void test_parser_reports_invalid_packets_without_emitting_messages() {
  const uint8_t packet[] = {0x80, 0x81, 0x90, 60};
  CapturingMessageSink sink;

  TEST_ASSERT_FALSE(BleMidiPacketParser::parse(packet, sizeof(packet), sink));
  TEST_ASSERT_EQUAL_UINT32(0, sink.count);
}

void test_parser_rejects_unsupported_midi_messages() {
  const uint8_t packet[] = {0x80, 0x81, 0xc0, 10};
  CapturingMessageSink sink;

  TEST_ASSERT_FALSE(BleMidiPacketParser::parse(packet, sizeof(packet), sink));
  TEST_ASSERT_EQUAL_UINT32(0, sink.count);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_parser_decodes_note_on_with_timestamp);
  RUN_TEST(test_parser_decodes_running_status_messages);
  RUN_TEST(test_parser_decodes_control_change_and_pitch_bend);
  RUN_TEST(test_parser_reports_invalid_packets_without_emitting_messages);
  RUN_TEST(test_parser_rejects_unsupported_midi_messages);
  return UNITY_END();
}
