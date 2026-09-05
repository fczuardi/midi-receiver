#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

struct ActiveNote {
  ActiveNote() = default;
  ActiveNote(uint8_t channelValue, uint8_t noteValue)
      : channel(channelValue), note(noteValue) {
  }

  uint8_t channel = 0;
  uint8_t note = 0;
};

class ActiveNotes {
public:
  static constexpr size_t MAX_ACTIVE_NOTES = 32;

  // Mark a note active. Returns true when the active-note set changed.
  bool noteOn(uint8_t channel, uint8_t note);

  // Mark a note inactive. Returns true when the active-note set changed.
  bool noteOff(uint8_t channel, uint8_t note);

  // Clear every active note, for example after a BLE disconnection.
  void clear();

  // Return whether the exact (channel, note) pair is currently active.
  bool contains(uint8_t channel, uint8_t note) const;

  // Return how many notes are currently active.
  size_t count() const;

  // Return an active note by display/order index. Caller must pass index < count().
  ActiveNote noteAt(size_t index) const;

private:
  size_t find(uint8_t channel, uint8_t note) const;

  std::array<ActiveNote, MAX_ACTIVE_NOTES> notes_{};
  size_t count_ = 0;
};
