#include "AppState.h"

BleConnectionState AppState::bleConnectionState() const {
  return bleConnectionState_;
}

uint32_t AppState::uptimeSeconds() const {
  return uptimeSeconds_;
}

uint32_t AppState::receivedMidiPacketCount() const {
  return receivedMidiPacketCount_;
}

size_t AppState::lastMidiPacketSize() const {
  return lastMidiPacketSize_;
}

uint32_t AppState::lastMidiPacketAtMs() const {
  return lastMidiPacketAtMs_;
}

bool AppState::fullDisplayRefreshNeeded() const {
  return fullDisplayRefreshNeeded_;
}

bool AppState::uptimeRefreshNeeded() const {
  return uptimeRefreshNeeded_;
}

bool AppState::midiActivityRefreshNeeded() const {
  return midiActivityRefreshNeeded_;
}

void AppState::setBleConnectionState(BleConnectionState state) {
  if (bleConnectionState_ == state) {
    return;
  }

  bleConnectionState_ = state;
  fullDisplayRefreshNeeded_ = true;
}

void AppState::setUptimeSeconds(uint32_t uptimeSeconds) {
  if (uptimeSeconds_ == uptimeSeconds) {
    return;
  }

  uptimeSeconds_ = uptimeSeconds;
  uptimeRefreshNeeded_ = true;
}

void AppState::recordMidiPacketActivity(size_t packetSize, uint32_t packetReceivedAtMs) {
  receivedMidiPacketCount_ += 1;
  lastMidiPacketSize_ = packetSize;
  lastMidiPacketAtMs_ = packetReceivedAtMs;
  midiActivityRefreshNeeded_ = true;
}

void AppState::markDisplayRefreshed() {
  fullDisplayRefreshNeeded_ = false;
  uptimeRefreshNeeded_ = false;
  midiActivityRefreshNeeded_ = false;
}
