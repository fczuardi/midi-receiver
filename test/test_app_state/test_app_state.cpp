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

void test_note_event_updates_active_notes() {
  AppState appState;
  const NoteEvent noteOn = makeNoteEvent(NoteEventType::NoteOn, 1, 60, 96);
  const NoteEvent noteOff = makeNoteEvent(NoteEventType::NoteOff, 1, 60, 64);

  appState.recordNoteEvent(noteOn, 100);

  TEST_ASSERT_EQUAL(1, appState.activeNoteCount());
  TEST_ASSERT_EQUAL(MidiActivityKind::NoteOn, appState.lastMidiActivityKind());
  TEST_ASSERT_EQUAL(1, appState.lastMidiChannel());
  TEST_ASSERT_EQUAL(60, appState.lastMidiNote());
  TEST_ASSERT_EQUAL(96, appState.lastMidiVelocity());

  appState.recordNoteEvent(noteOff, 200);

  TEST_ASSERT_EQUAL(0, appState.activeNoteCount());
  TEST_ASSERT_EQUAL(MidiActivityKind::NoteOff, appState.lastMidiActivityKind());
  TEST_ASSERT_EQUAL(60, appState.lastMidiNote());
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

void test_control_change_records_last_controller() {
  AppState appState;

  TEST_ASSERT_FALSE(appState.hasControlChange());

  appState.recordControlChange(1, 1, 83, 300);

  TEST_ASSERT_EQUAL(MidiActivityKind::ControlChange, appState.lastMidiActivityKind());
  TEST_ASSERT_TRUE(appState.hasControlChange());
  TEST_ASSERT_EQUAL(1, appState.lastControlChangeChannel());
  TEST_ASSERT_EQUAL(1, appState.lastControlChangeNumber());
  TEST_ASSERT_EQUAL(83, appState.lastControlChangeValue());
  TEST_ASSERT_FALSE(appState.sustainEnabled());
}

void test_sustain_control_change_tracks_hold_state() {
  AppState appState;

  appState.recordControlChange(1, 64, 127, 300);
  TEST_ASSERT_TRUE(appState.sustainEnabled());

  appState.recordControlChange(1, 64, 0, 400);
  TEST_ASSERT_FALSE(appState.sustainEnabled());
}

void test_pitch_bend_records_centered_value() {
  AppState appState;

  TEST_ASSERT_FALSE(appState.hasPitchBend());

  appState.recordPitchBend(1, -1200, 500);

  TEST_ASSERT_EQUAL(MidiActivityKind::PitchBend, appState.lastMidiActivityKind());
  TEST_ASSERT_TRUE(appState.hasPitchBend());
  TEST_ASSERT_EQUAL(1, appState.lastPitchBendChannel());
  TEST_ASSERT_EQUAL(-1200, appState.lastPitchBendValue());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_note_on_with_velocity_zero_is_treated_as_note_off);
  RUN_TEST(test_note_event_updates_active_notes);
  RUN_TEST(test_disconnect_clear_removes_active_notes);
  RUN_TEST(test_active_note_overflow_is_counted);
  RUN_TEST(test_control_change_records_last_controller);
  RUN_TEST(test_sustain_control_change_tracks_hold_state);
  RUN_TEST(test_pitch_bend_records_centered_value);
  return UNITY_END();
}
