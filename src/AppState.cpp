#include "AppState.h"

BleConnectionState AppState::bleConnectionState() const {
  return bleConnectionState_;
}

uint32_t AppState::uptimeSeconds() const {
  return uptimeSeconds_;
}

uint32_t AppState::receivedMidiMessageCount() const {
  return receivedMidiMessageCount_;
}

uint32_t AppState::activeSensingMessageCount() const {
  return activeSensingMessageCount_;
}

MidiActivityKind AppState::lastMidiActivityKind() const {
  return lastMidiActivityKind_;
}

uint8_t AppState::lastMidiChannel() const {
  return lastMidiChannel_;
}

uint8_t AppState::lastMidiNote() const {
  return lastMidiNote_;
}

uint8_t AppState::lastMidiVelocity() const {
  return lastMidiVelocity_;
}

uint32_t AppState::lastMidiActivityAtMs() const {
  return lastMidiActivityAtMs_;
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

void AppState::recordMidiActivity(
    MidiActivityKind kind,
    uint8_t channel,
    uint8_t note,
    uint8_t velocity,
    uint32_t activityAtMs) {
  receivedMidiMessageCount_ += 1;
  lastMidiActivityKind_ = kind;
  lastMidiChannel_ = channel;
  lastMidiNote_ = note;
  lastMidiVelocity_ = velocity;
  lastMidiActivityAtMs_ = activityAtMs;

  if (kind == MidiActivityKind::ActiveSensing) {
    activeSensingMessageCount_ += 1;
  }

  midiActivityRefreshNeeded_ = true;
}

void AppState::markDisplayRefreshed() {
  fullDisplayRefreshNeeded_ = false;
  uptimeRefreshNeeded_ = false;
  midiActivityRefreshNeeded_ = false;
}
