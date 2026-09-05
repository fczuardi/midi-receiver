#include <unity.h>

#include "ActiveNotes.h"

void test_note_on_adds_active_note() {
  ActiveNotes activeNotes;

  TEST_ASSERT_TRUE(activeNotes.noteOn(1, 48));
  TEST_ASSERT_EQUAL(1, activeNotes.count());
  TEST_ASSERT_TRUE(activeNotes.contains(1, 48));
}

void test_duplicate_note_on_does_not_double_count() {
  ActiveNotes activeNotes;

  TEST_ASSERT_TRUE(activeNotes.noteOn(1, 48));
  TEST_ASSERT_FALSE(activeNotes.noteOn(1, 48));
  TEST_ASSERT_EQUAL(1, activeNotes.count());
}

void test_same_note_on_different_channels_counts_separately() {
  ActiveNotes activeNotes;

  TEST_ASSERT_TRUE(activeNotes.noteOn(1, 48));
  TEST_ASSERT_TRUE(activeNotes.noteOn(2, 48));
  TEST_ASSERT_EQUAL(2, activeNotes.count());
}

void test_note_off_removes_active_note() {
  ActiveNotes activeNotes;

  activeNotes.noteOn(1, 48);

  TEST_ASSERT_TRUE(activeNotes.noteOff(1, 48));
  TEST_ASSERT_EQUAL(0, activeNotes.count());
  TEST_ASSERT_FALSE(activeNotes.contains(1, 48));
}

void test_unknown_note_off_does_not_change_count() {
  ActiveNotes activeNotes;

  activeNotes.noteOn(1, 48);

  TEST_ASSERT_FALSE(activeNotes.noteOff(1, 52));
  TEST_ASSERT_EQUAL(1, activeNotes.count());
  TEST_ASSERT_TRUE(activeNotes.contains(1, 48));
}

void test_chord_with_eight_notes_is_tracked() {
  ActiveNotes activeNotes;

  for (uint8_t note = 48; note < 56; ++note) {
    TEST_ASSERT_TRUE(activeNotes.noteOn(1, note));
  }

  TEST_ASSERT_EQUAL(8, activeNotes.count());
  for (uint8_t note = 48; note < 56; ++note) {
    TEST_ASSERT_TRUE(activeNotes.contains(1, note));
  }
}

void test_clear_removes_all_active_notes() {
  ActiveNotes activeNotes;

  activeNotes.noteOn(1, 48);
  activeNotes.noteOn(1, 52);
  activeNotes.noteOn(1, 55);

  activeNotes.clear();

  TEST_ASSERT_EQUAL(0, activeNotes.count());
  TEST_ASSERT_FALSE(activeNotes.contains(1, 48));
  TEST_ASSERT_FALSE(activeNotes.contains(1, 52));
  TEST_ASSERT_FALSE(activeNotes.contains(1, 55));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_note_on_adds_active_note);
  RUN_TEST(test_duplicate_note_on_does_not_double_count);
  RUN_TEST(test_same_note_on_different_channels_counts_separately);
  RUN_TEST(test_note_off_removes_active_note);
  RUN_TEST(test_unknown_note_off_does_not_change_count);
  RUN_TEST(test_chord_with_eight_notes_is_tracked);
  RUN_TEST(test_clear_removes_all_active_notes);
  return UNITY_END();
}
