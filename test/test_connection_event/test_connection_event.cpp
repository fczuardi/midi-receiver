#include <unity.h>

#include "InstrumentEventSink.h"

class CapturingInstrumentEventSink : public InstrumentEventSink {
public:
  void onNoteEvent(const NoteEvent&) override {
  }

  void onPitchBendEvent(const PitchBendEvent&) override {
  }

  void onDisconnected() override {
    disconnected = true;
  }

  bool disconnected = false;
};

void test_shared_instrument_event_sink_receives_disconnection() {
  CapturingInstrumentEventSink sink;

  sink.onDisconnected();

  TEST_ASSERT_TRUE(sink.disconnected);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_shared_instrument_event_sink_receives_disconnection);
  return UNITY_END();
}
