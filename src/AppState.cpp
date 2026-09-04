#include "AppState.h"

BleConnectionState AppState::bleConnectionState() const {
  return bleConnectionState_;
}

uint32_t AppState::uptimeSeconds() const {
  return uptimeSeconds_;
}

bool AppState::displayRefreshNeeded() const {
  return displayRefreshNeeded_;
}

void AppState::setBleConnectionState(BleConnectionState state) {
  if (bleConnectionState_ == state) {
    return;
  }

  bleConnectionState_ = state;
  displayRefreshNeeded_ = true;
}

void AppState::setUptimeSeconds(uint32_t uptimeSeconds) {
  if (uptimeSeconds_ == uptimeSeconds) {
    return;
  }

  uptimeSeconds_ = uptimeSeconds;
  displayRefreshNeeded_ = true;
}

void AppState::markDisplayRefreshed() {
  displayRefreshNeeded_ = false;
}

