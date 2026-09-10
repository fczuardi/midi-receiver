#include <unity.h>

#include <array>

#include "BleMidiPacketDecoder.h"

class CapturingMessageSink : public MidiMessageSink {
public:
  void onMidiMessage(const MidiMessage& message) override {
    messages[count] = message;
    count += 1;
  }

  std::array<MidiMessage, 8> messages{};
  size_t count = 0;
};

void test_decoder_removes_timestamps_and_decodes_messages() {
  const uint8_t packet[] = {0x80, 0x81, 0x90, 60, 100, 0x82, 64, 80};
  CapturingMessageSink sink;

  TEST_ASSERT_TRUE(
      BleMidiPacketDecoder::parse(packet, sizeof(packet), sink));
  TEST_ASSERT_EQUAL_UINT32(2, sink.count);
  TEST_ASSERT_EQUAL_UINT8(60, sink.messages[0].data1);
  TEST_ASSERT_EQUAL_UINT8(64, sink.messages[1].data1);
}

void test_decoder_rejects_bad_header_and_truncated_message() {
  const uint8_t badHeader[] = {0x00, 0x81, 0x90, 60, 100};
  const uint8_t truncated[] = {0x80, 0x81, 0x90, 60};
  CapturingMessageSink sink;

  TEST_ASSERT_FALSE(
      BleMidiPacketDecoder::parse(badHeader, sizeof(badHeader), sink));
  TEST_ASSERT_FALSE(
      BleMidiPacketDecoder::parse(truncated, sizeof(truncated), sink));
  TEST_ASSERT_EQUAL_UINT32(0, sink.count);
}

void test_decoder_rejects_packet_ending_after_timestamp() {
  const uint8_t timestampOnly[] = {0x80, 0x81, 0x82};
  CapturingMessageSink sink;

  TEST_ASSERT_FALSE(
      BleMidiPacketDecoder::parse(timestampOnly, sizeof(timestampOnly), sink));
  TEST_ASSERT_EQUAL_UINT32(0, sink.count);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_decoder_removes_timestamps_and_decodes_messages);
  RUN_TEST(test_decoder_rejects_bad_header_and_truncated_message);
  RUN_TEST(test_decoder_rejects_packet_ending_after_timestamp);
  return UNITY_END();
}
