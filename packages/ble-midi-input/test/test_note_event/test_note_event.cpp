#include <unity.h>

#include "InstrumentEventSink.h"
#include "MidiNoteEventFactory.h"

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

class CapturingInstrumentEventSink : public InstrumentEventSink {
public:
  void onNoteEvent(const NoteEvent& event) override {
    received = true;
    lastEvent = event;
  }

  void onPitchBendEvent(const PitchBendEvent& event) override {
    receivedPitchBend = true;
    lastPitchBendEvent = event;
  }

  void onControlChangeEvent(const ControlChangeEvent& event) override {
    receivedControlChange = true;
    lastControlChangeEvent = event;
  }

  void onDisconnected() override {
    disconnected = true;
  }

  bool received = false;
  bool receivedPitchBend = false;
  bool receivedControlChange = false;
  bool disconnected = false;
  NoteEvent lastEvent = {NoteEventType::NoteOff, 0, 0, 0};
  PitchBendEvent lastPitchBendEvent = {0, 0};
  ControlChangeEvent lastControlChangeEvent = {0, 0, 0};
};

void test_shared_instrument_event_sink_receives_note_event() {
  CapturingInstrumentEventSink sink;
  const NoteEvent event = makeNoteEvent(NoteEventType::NoteOn, 4, 72, 100);

  sink.onNoteEvent(event);

  TEST_ASSERT_TRUE(sink.received);
  TEST_ASSERT_EQUAL(NoteEventType::NoteOn, sink.lastEvent.type);
  TEST_ASSERT_EQUAL_UINT8(4, sink.lastEvent.channel);
  TEST_ASSERT_EQUAL_UINT8(72, sink.lastEvent.note);
  TEST_ASSERT_EQUAL_UINT8(100, sink.lastEvent.velocity);
}

void test_shared_instrument_event_sink_receives_pitch_bend_event() {
  CapturingInstrumentEventSink sink;
  const PitchBendEvent event = {5, -1234};

  sink.onPitchBendEvent(event);

  TEST_ASSERT_TRUE(sink.receivedPitchBend);
  TEST_ASSERT_EQUAL_UINT8(5, sink.lastPitchBendEvent.channel);
  TEST_ASSERT_EQUAL_INT16(-1234, sink.lastPitchBendEvent.value);
}

void test_shared_instrument_event_sink_receives_control_change_event() {
  CapturingInstrumentEventSink sink;
  const ControlChangeEvent event = {6, 1, 96};

  sink.onControlChangeEvent(event);

  TEST_ASSERT_TRUE(sink.receivedControlChange);
  TEST_ASSERT_EQUAL_UINT8(6, sink.lastControlChangeEvent.channel);
  TEST_ASSERT_EQUAL_UINT8(1, sink.lastControlChangeEvent.controller);
  TEST_ASSERT_EQUAL_UINT8(96, sink.lastControlChangeEvent.value);
}

void test_shared_instrument_event_sink_receives_disconnection() {
  CapturingInstrumentEventSink sink;

  sink.onDisconnected();

  TEST_ASSERT_TRUE(sink.disconnected);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_note_on_event_preserves_parsed_fields);
  RUN_TEST(test_note_off_event_preserves_parsed_fields);
  RUN_TEST(test_note_on_with_velocity_zero_becomes_note_off);
  RUN_TEST(test_shared_instrument_event_sink_receives_note_event);
  RUN_TEST(test_shared_instrument_event_sink_receives_pitch_bend_event);
  RUN_TEST(test_shared_instrument_event_sink_receives_control_change_event);
  RUN_TEST(test_shared_instrument_event_sink_receives_disconnection);
  return UNITY_END();
}
