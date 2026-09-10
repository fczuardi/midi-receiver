#include "PendingMidiEventQueue.h"

bool PendingMidiEventQueue::push(const PendingMidiEvent& event) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (count_ >= events_.size()) {
    droppedCount_.fetch_add(1);
    return false;
  }

  events_[count_] = event;
  count_ += 1;
  return true;
}

size_t PendingMidiEventQueue::drain(
    std::array<PendingMidiEvent, Capacity>& destination) {
  std::lock_guard<std::mutex> lock(mutex_);
  const size_t drainedCount = count_;
  destination = events_;
  count_ = 0;
  return drainedCount;
}

uint32_t PendingMidiEventQueue::takeDroppedCount() {
  return droppedCount_.exchange(0);
}

void PendingMidiEventQueue::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  count_ = 0;
  droppedCount_.store(0);
}
