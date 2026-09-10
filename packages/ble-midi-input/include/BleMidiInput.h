#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>

#include "InstrumentEventSink.h"

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

  enum class PendingMidiEventKind : uint8_t {
    None,
    NoteOn,
    NoteOff,
    ControlChange,
    PitchBend,
  };

  struct PendingMidiEvent {
    PendingMidiEventKind kind = PendingMidiEventKind::None;
    uint8_t channel = 0;
    uint8_t data1 = 0;
    uint8_t data2 = 0;
    int bendValue = 0;
    uint32_t activityAtMs = 0;
  };

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
  void parseBleMidiPacket(uint8_t* data, size_t size);
  bool parseMidiMessage(uint8_t status, uint8_t*& cursor, uint8_t* end);
  void applyPendingMidiActivity();
  void discardPendingMidiActivity();
  bool enqueuePendingMidiEvent(const PendingMidiEvent& event);

  InstrumentEventSink* instrumentEventSink_ = nullptr;
  BleMidiInputDiagnosticSink* diagnosticSink_ = nullptr;
  bool connected_ = false;
  std::atomic<bool> connectionStarted_{false};
  std::atomic<bool> connectionEnded_{false};
  std::atomic<uint32_t> droppedPendingMidiEventCount_{0};

  static constexpr size_t MAX_PENDING_MIDI_EVENTS = 32;

  std::mutex pendingMidiEventMutex_;
  std::array<PendingMidiEvent, MAX_PENDING_MIDI_EVENTS> pendingMidiEvents_{};
  size_t pendingMidiEventCount_ = 0;
};
