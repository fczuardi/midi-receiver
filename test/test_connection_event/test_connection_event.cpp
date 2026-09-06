#include <unity.h>

#include "ConnectionEvent.h"

class CapturingConnectionEventSink : public ConnectionEventSink {
public:
  void onDisconnected() override {
    disconnected = true;
  }

  bool disconnected = false;
};

void test_notify_disconnected_delivers_event() {
  CapturingConnectionEventSink sink;

  notifyDisconnected(&sink);

  TEST_ASSERT_TRUE(sink.disconnected);
}

void test_notify_disconnected_allows_null_sink() {
  notifyDisconnected(nullptr);

  TEST_PASS();
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_notify_disconnected_delivers_event);
  RUN_TEST(test_notify_disconnected_allows_null_sink);
  return UNITY_END();
}
