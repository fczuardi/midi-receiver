#include <unity.h>

#include "NoteEvent.h"

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

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_note_on_event_preserves_parsed_fields);
  RUN_TEST(test_note_off_event_preserves_parsed_fields);
  RUN_TEST(test_note_on_with_velocity_zero_becomes_note_off);
  return UNITY_END();
}
