#pragma once

#include <array>
#include <atomic>
#include <mutex>

#include "AppState.h"
#include "ConnectionEvent.h"
#include "NoteEvent.h"

// Owns BLE setup for this firmware.
//
// This class wraps the Arduino BLE-MIDI library so the rest of the app does not
// depend directly on its global MIDI objects and callback API.
class BleMidiPeripheral {
public:
  explicit BleMidiPeripheral(AppState& appState);

  // Initialize the BLE stack, create the MIDI GATT service, and advertise it.
  void begin();

  // Apply BLE events that were observed by callbacks.
  // Keeping state changes here makes the main loop easier to inspect.
  void update();

  // Register an optional typed note-event consumer.
  // The sink is not owned by BleMidiPeripheral and must outlive it.
  void setNoteEventSink(NoteEventSink* sink);

  // Register an optional connection lifecycle consumer.
  // The sink is not owned by BleMidiPeripheral and must outlive it.
  void setConnectionEventSink(ConnectionEventSink* sink);

private:
  struct PendingMidiEvent {
    MidiActivityKind kind = MidiActivityKind::None;
    uint8_t channel = 0;
    uint8_t data1 = 0;
    uint8_t data2 = 0;
    int bendValue = 0;
    uint32_t activityAtMs = 0;
  };

  static void handleConnected();
  static void handleDisconnected();
  static void handleActiveSensing();
  static void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  static void handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);
  static void handleControlChange(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue);
  static void handlePitchBend(uint8_t channel, int bendValue);

  static BleMidiPeripheral* activeInstance_;

  void noteReceived(MidiActivityKind kind, uint8_t channel, uint8_t note, uint8_t velocity);
  void controlChangeReceived(
      uint8_t channel,
      uint8_t controllerNumber,
      uint8_t controllerValue);
  void pitchBendReceived(uint8_t channel, int bendValue);
  void activeSensingReceived();
  void applyPendingMidiActivity();
  void discardPendingMidiActivity();
  bool enqueuePendingMidiEvent(const PendingMidiEvent& event);

  AppState& appState_;
  NoteEventSink* noteEventSink_ = nullptr;
  ConnectionEventSink* connectionEventSink_ = nullptr;
  bool connected_ = false;
  std::atomic<bool> connectionStarted_{false};
  std::atomic<bool> connectionEnded_{false};
  std::atomic<uint32_t> pendingActiveSensingCount_{0};
  std::atomic<uint32_t> pendingMidiActivityAtMs_{0};
  std::atomic<uint32_t> droppedPendingMidiEventCount_{0};

  static constexpr size_t MAX_PENDING_MIDI_EVENTS = 32;

  std::mutex pendingMidiEventMutex_;
  std::array<PendingMidiEvent, MAX_PENDING_MIDI_EVENTS> pendingMidiEvents_{};
  size_t pendingMidiEventCount_ = 0;
};
