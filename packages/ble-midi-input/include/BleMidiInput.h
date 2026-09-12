#pragma once

#include <atomic>
#include <cstdint>

#include "InstrumentEventSink.h"
#include "PendingMidiEventQueue.h"

class BleMidiCharacteristicCallbacks;
class BleMidiServerCallbacks;

// Optional diagnostics observer for the BLE MIDI input boundary.
//
// Instrument consumers should use InstrumentEventSink. This observer exists so
// the receiver firmware can keep its display and serial diagnostics without
// making the reusable BLE input depend on AppState.
class BleMidiInputDiagnosticSink {
public:
  virtual ~BleMidiInputDiagnosticSink() = default;

  virtual void onBleMidiAdvertising(const char* deviceName) = 0;
  virtual void onBleMidiConnected() = 0;
  virtual void onBleMidiDisconnected() = 0;
  virtual void onBleMidiNoteEvent(
      const NoteEvent& event,
      uint32_t activityAtMs) = 0;
  virtual void onBleMidiControlChange(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue,
      uint32_t activityAtMs) = 0;
  virtual void onBleMidiPitchBend(
      uint8_t channel,
      int bendValue,
      uint32_t activityAtMs) = 0;
  // Reports rejected MIDI status bytes for receiver diagnostics. These bytes
  // are not queued or delivered to InstrumentEventSink.
  virtual void onBleMidiUnsupportedStatus(uint8_t statusByte) {
  }
  virtual void onBleMidiDroppedEvents(uint32_t droppedEventCount) = 0;
};

// BLE MIDI transport that produces shared instrument events.
//
// This class owns BLE-MIDI callbacks, pending event buffering, and MIDI note
// normalization. It does not know about display state, AppState, or audio.
//
// Only one BleMidiInput instance may be active. The underlying BLE-MIDI library
// is configured through global objects, so callbacks are routed through a single
// active instance pointer.
class BleMidiInput {
public:
  // Initialize the BLE stack, create the MIDI GATT service, and advertise it.
  // Calling begin() more than once has no effect.
  void begin();

  // Apply connection changes and drain MIDI events captured by BLE callbacks.
  void update();

  // Register an optional instrument-event consumer.
  // The sink is not owned by BleMidiInput and must outlive it.
  void setInstrumentEventSink(InstrumentEventSink* sink);

  // Register an optional diagnostics observer.
  // The sink is not owned by BleMidiInput and must outlive it.
  void setDiagnosticSink(BleMidiInputDiagnosticSink* sink);

private:
  friend class BleMidiCharacteristicCallbacks;
  friend class BleMidiServerCallbacks;
  class ParsedMessageSink;

  static void handleConnected();
  static void handleDisconnected();

  static BleMidiInput* activeInstance_;

  void noteReceived(
      PendingMidiEventKind kind,
      uint8_t channel,
      uint8_t note,
      uint8_t velocity);
  void controlChangeReceived(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue);
  void pitchBendReceived(uint8_t channel, int bendValue);
  void unsupportedStatusReceived(uint8_t statusByte);
  void parseBleMidiPacket(const uint8_t* data, size_t size);
  void applyPendingMidiActivity();
  void discardPendingMidiActivity();

  InstrumentEventSink* instrumentEventSink_ = nullptr;
  BleMidiInputDiagnosticSink* diagnosticSink_ = nullptr;
  bool begun_ = false;
  bool connected_ = false;
  std::atomic<bool> connectionStarted_{false};
  std::atomic<bool> connectionEnded_{false};
  PendingMidiEventQueue pendingMidiEvents_;
};
