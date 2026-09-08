#include <Arduino.h>

#include "BleMidiInput.h"
#include "PitchBendEvent.h"

class CapturingInstrumentEventSink : public InstrumentEventSink {
public:
  void onNoteEvent(const NoteEvent&) override {
  }

  void onPitchBendEvent(const PitchBendEvent&) override {
  }

  void onDisconnected() override {
  }
};

CapturingInstrumentEventSink sink;
BleMidiInput input;

void setup() {
  input.setInstrumentEventSink(&sink);
}

void loop() {
  input.update();
}
