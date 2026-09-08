#include "ActiveNotes.h"

namespace {
constexpr size_t NOT_FOUND = ActiveNotes::MAX_ACTIVE_NOTES;
}

bool ActiveNotes::noteOn(uint8_t channel, uint8_t note) {
  if (contains(channel, note)) {
    return false;
  }

  if (count_ >= notes_.size()) {
    return false;
  }

  notes_[count_] = ActiveNote{channel, note};
  count_ += 1;
  return true;
}

bool ActiveNotes::noteOff(uint8_t channel, uint8_t note) {
  const size_t index = find(channel, note);
  if (index == NOT_FOUND) {
    return false;
  }

  // Keep the array compact by moving the last active note into the removed slot.
  count_ -= 1;
  notes_[index] = notes_[count_];
  notes_[count_] = ActiveNote{};
  return true;
}

void ActiveNotes::clear() {
  for (size_t index = 0; index < count_; ++index) {
    notes_[index] = ActiveNote{};
  }
  count_ = 0;
}

bool ActiveNotes::contains(uint8_t channel, uint8_t note) const {
  return find(channel, note) != NOT_FOUND;
}

size_t ActiveNotes::count() const {
  return count_;
}

ActiveNote ActiveNotes::noteAt(size_t index) const {
  return notes_[index];
}

size_t ActiveNotes::find(uint8_t channel, uint8_t note) const {
  for (size_t index = 0; index < count_; ++index) {
    if (notes_[index].channel == channel && notes_[index].note == note) {
      return index;
    }
  }

  return NOT_FOUND;
}
