#include <unity.h>

#include "NoteEvent.h"

class CapturingNoteEventSink : public NoteEventSink {
public:
  void onNoteEvent(const NoteEvent& event) override {
    received = true;
    lastEvent = event;
  }

  bool received = false;
  NoteEvent lastEvent = {NoteEventType::NoteOff, 0, 0, 0};
};

void test_note_on_event_preserves_parsed_fields() {
  const NoteEvent event = makeNoteEvent(NoteEventType::NoteOn, 1, 60, 96);

  TEST_ASSERT_EQUAL(NoteEventType::NoteOn, event.type);
  TEST_ASSERT_EQUAL_UINT8(1, event.channel);
  TEST_ASSERT_EQUAL_UINT8(60, event.note);
  TEST_ASSERT_EQUAL_UINT8(96, event.velocity);
}

void test_note_off_event_preserves_parsed_fields() {
  const NoteEvent event = makeNoteEvent(NoteEventType::NoteOff, 2, 64, 80);

  TEST_ASSERT_EQUAL(NoteEventType::NoteOff, event.type);
  TEST_ASSERT_EQUAL_UINT8(2, event.channel);
  TEST_ASSERT_EQUAL_UINT8(64, event.note);
  TEST_ASSERT_EQUAL_UINT8(80, event.velocity);
}

void test_note_on_with_velocity_zero_becomes_note_off() {
  const NoteEvent event = makeNoteEvent(NoteEventType::NoteOn, 3, 67, 0);

  TEST_ASSERT_EQUAL(NoteEventType::NoteOff, event.type);
  TEST_ASSERT_EQUAL_UINT8(3, event.channel);
  TEST_ASSERT_EQUAL_UINT8(67, event.note);
  TEST_ASSERT_EQUAL_UINT8(0, event.velocity);
}

void test_notify_note_event_sink_delivers_event() {
  CapturingNoteEventSink sink;
  const NoteEvent event = makeNoteEvent(NoteEventType::NoteOn, 4, 72, 100);

  notifyNoteEventSink(&sink, event);

  TEST_ASSERT_TRUE(sink.received);
  TEST_ASSERT_EQUAL(NoteEventType::NoteOn, sink.lastEvent.type);
  TEST_ASSERT_EQUAL_UINT8(4, sink.lastEvent.channel);
  TEST_ASSERT_EQUAL_UINT8(72, sink.lastEvent.note);
  TEST_ASSERT_EQUAL_UINT8(100, sink.lastEvent.velocity);
}

void test_notify_note_event_sink_allows_null_sink() {
  const NoteEvent event = makeNoteEvent(NoteEventType::NoteOn, 4, 72, 100);

  notifyNoteEventSink(nullptr, event);

  TEST_PASS();
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_note_on_event_preserves_parsed_fields);
  RUN_TEST(test_note_off_event_preserves_parsed_fields);
  RUN_TEST(test_note_on_with_velocity_zero_becomes_note_off);
  RUN_TEST(test_notify_note_event_sink_delivers_event);
  RUN_TEST(test_notify_note_event_sink_allows_null_sink);
  return UNITY_END();
}
