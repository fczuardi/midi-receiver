#include <unity.h>

#include <array>

#include "PendingMidiEventQueue.h"

PendingMidiEvent eventFor(uint8_t note) {
  PendingMidiEvent event;
  event.kind = PendingMidiEventKind::NoteOn;
  event.channel = 2;
  event.data1 = note;
  event.data2 = 100;
  return event;
}

void test_queue_preserves_order_when_drained() {
  PendingMidiEventQueue queue;
  queue.push(eventFor(60));
  queue.push(eventFor(64));
  std::array<PendingMidiEvent, PendingMidiEventQueue::Capacity> events{};

  TEST_ASSERT_EQUAL_UINT32(2, queue.drain(events));
  TEST_ASSERT_EQUAL_UINT8(60, events[0].data1);
  TEST_ASSERT_EQUAL_UINT8(64, events[1].data1);
  TEST_ASSERT_EQUAL_UINT32(0, queue.drain(events));
}

void test_queue_rejects_events_after_capacity_and_counts_drops() {
  PendingMidiEventQueue queue;

  for (size_t index = 0; index < PendingMidiEventQueue::Capacity; ++index) {
    TEST_ASSERT_TRUE(queue.push(eventFor(static_cast<uint8_t>(index))));
  }

  TEST_ASSERT_FALSE(queue.push(eventFor(99)));
  TEST_ASSERT_FALSE(queue.push(eventFor(100)));
  TEST_ASSERT_EQUAL_UINT32(2, queue.takeDroppedCount());
  TEST_ASSERT_EQUAL_UINT32(0, queue.takeDroppedCount());
}

void test_queue_clear_discards_pending_events_and_drop_count() {
  PendingMidiEventQueue queue;
  std::array<PendingMidiEvent, PendingMidiEventQueue::Capacity> events{};

  queue.push(eventFor(60));
  for (size_t index = 0; index < PendingMidiEventQueue::Capacity; ++index) {
    queue.push(eventFor(static_cast<uint8_t>(index)));
  }
  queue.clear();

  TEST_ASSERT_EQUAL_UINT32(0, queue.drain(events));
  TEST_ASSERT_EQUAL_UINT32(0, queue.takeDroppedCount());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_queue_preserves_order_when_drained);
  RUN_TEST(test_queue_rejects_events_after_capacity_and_counts_drops);
  RUN_TEST(test_queue_clear_discards_pending_events_and_drop_count);
  return UNITY_END();
}
