#include <unity.h>

#include "MidiNoteName.h"

void test_known_note_names_use_middle_c_as_c4() {
  TEST_ASSERT_EQUAL_STRING("C-1", midiNoteName(0).data());
  TEST_ASSERT_EQUAL_STRING("C0", midiNoteName(12).data());
  TEST_ASSERT_EQUAL_STRING("C4", midiNoteName(60).data());
  TEST_ASSERT_EQUAL_STRING("C#4", midiNoteName(61).data());
  TEST_ASSERT_EQUAL_STRING("A4", midiNoteName(69).data());
  TEST_ASSERT_EQUAL_STRING("G9", midiNoteName(127).data());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_known_note_names_use_middle_c_as_c4);
  return UNITY_END();
}
