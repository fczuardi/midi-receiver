#pragma once

#include <array>
#include <atomic>
#include <mutex>

#include "AppState.h"

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

private:
  struct PendingNoteEvent {
    MidiActivityKind kind = MidiActivityKind::None;
    uint8_t channel = 0;
    uint8_t note = 0;
    uint8_t velocity = 0;
    uint32_t activityAtMs = 0;
  };

  static void handleConnected();
  static void handleDisconnected();
  static void handleActiveSensing();
  static void handleNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
  static void handleNoteOff(uint8_t channel, uint8_t note, uint8_t velocity);

  static BleMidiPeripheral* activeInstance_;

  void noteReceived(MidiActivityKind kind, uint8_t channel, uint8_t note, uint8_t velocity);
  void activeSensingReceived();
  void applyPendingMidiActivity();
  bool enqueuePendingNoteEvent(const PendingNoteEvent& event);

  AppState& appState_;
  std::atomic<bool> connectionStarted_{false};
  std::atomic<bool> connectionEnded_{false};
  std::atomic<uint32_t> pendingActiveSensingCount_{0};
  std::atomic<uint32_t> pendingMidiActivityAtMs_{0};
  std::atomic<uint32_t> droppedPendingNoteEventCount_{0};

  static constexpr size_t MAX_PENDING_NOTE_EVENTS = 32;

  std::mutex pendingNoteEventMutex_;
  std::array<PendingNoteEvent, MAX_PENDING_NOTE_EVENTS> pendingNoteEvents_{};
  size_t pendingNoteEventCount_ = 0;
};
