#include <unity.h>

#include "AppState.h"

void test_note_on_with_velocity_zero_is_treated_as_note_off() {
  AppState appState;

  appState.recordMidiActivity(MidiActivityKind::NoteOn, 1, 48, 64, 100);
  TEST_ASSERT_EQUAL(1, appState.activeNoteCount());
  TEST_ASSERT_TRUE(appState.activeNoteAt(0).note == 48);

  appState.recordMidiActivity(MidiActivityKind::NoteOn, 1, 48, 0, 200);

  TEST_ASSERT_EQUAL(0, appState.activeNoteCount());
  TEST_ASSERT_EQUAL(MidiActivityKind::NoteOff, appState.lastMidiActivityKind());
  TEST_ASSERT_EQUAL(48, appState.lastMidiNote());
  TEST_ASSERT_EQUAL(0, appState.lastMidiVelocity());
}

void test_disconnect_clear_removes_active_notes() {
  AppState appState;

  appState.recordMidiActivity(MidiActivityKind::NoteOn, 1, 48, 64, 100);
  appState.recordMidiActivity(MidiActivityKind::NoteOn, 1, 52, 64, 110);

  appState.clearActiveNotes();

  TEST_ASSERT_EQUAL(0, appState.activeNoteCount());
}

void test_active_note_overflow_is_counted() {
  AppState appState;

  for (uint8_t note = 0; note < ActiveNotes::MAX_ACTIVE_NOTES; ++note) {
    appState.recordMidiActivity(MidiActivityKind::NoteOn, 1, note, 64, 100 + note);
  }

  appState.recordMidiActivity(
      MidiActivityKind::NoteOn,
      1,
      ActiveNotes::MAX_ACTIVE_NOTES,
      64,
      200);

  TEST_ASSERT_EQUAL(ActiveNotes::MAX_ACTIVE_NOTES, appState.activeNoteCount());
  TEST_ASSERT_EQUAL(1, appState.droppedActiveNoteCount());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_note_on_with_velocity_zero_is_treated_as_note_off);
  RUN_TEST(test_disconnect_clear_removes_active_notes);
  RUN_TEST(test_active_note_overflow_is_counted);
  return UNITY_END();
}
