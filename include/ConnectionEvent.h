#pragma once

// Typed lifecycle notifications produced by the receiver boundary.
//
// This contract intentionally stays separate from NoteEvent: a disconnection is
// not a musical note, but consumers such as instruments may need to silence
// their state when the MIDI sender disappears.
class ConnectionEventSink {
public:
  virtual ~ConnectionEventSink() = default;

  virtual void onDisconnected() = 0;
};

// Deliver a disconnection notification to an optional sink.
void notifyDisconnected(ConnectionEventSink* sink);
