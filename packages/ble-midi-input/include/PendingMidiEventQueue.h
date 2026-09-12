#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>

enum class PendingMidiEventKind : uint8_t {
  None,
  NoteOn,
  NoteOff,
  ControlChange,
  PitchBend,
  UnsupportedStatus,
};

struct PendingMidiEvent {
  PendingMidiEventKind kind = PendingMidiEventKind::None;
  uint8_t channel = 0;
  uint8_t data1 = 0;
  uint8_t data2 = 0;
  int bendValue = 0;
  uint8_t dataSize = 0;
  uint32_t activityAtMs = 0;
};

// Fixed-capacity handoff between BLE callbacks and the application loop.
class PendingMidiEventQueue {
public:
  static constexpr size_t Capacity = 32;

  bool push(const PendingMidiEvent& event);
  size_t drain(
      std::array<PendingMidiEvent, Capacity>& destination);
  uint32_t takeDroppedCount();
  void clear();

private:
  std::mutex mutex_;
  std::array<PendingMidiEvent, Capacity> events_{};
  size_t count_ = 0;
  std::atomic<uint32_t> droppedCount_{0};
};
