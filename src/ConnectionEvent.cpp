#include "ConnectionEvent.h"

void notifyDisconnected(ConnectionEventSink* sink) {
  if (sink == nullptr) {
    return;
  }

  sink->onDisconnected();
}
