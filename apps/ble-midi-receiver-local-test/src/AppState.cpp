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

uint32_t AppState::droppedActiveNoteCount() const {
  return droppedActiveNoteCount_;
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

uint8_t AppState::lastControlChangeChannel() const {
  return lastControlChangeChannel_;
}

uint8_t AppState::lastControlChangeNumber() const {
  return lastControlChangeNumber_;
}

uint8_t AppState::lastControlChangeValue() const {
  return lastControlChangeValue_;
}

bool AppState::hasControlChange() const {
  return hasControlChange_;
}

bool AppState::sustainEnabled() const {
  return sustainEnabled_;
}

uint8_t AppState::lastPitchBendChannel() const {
  return lastPitchBendChannel_;
}

int AppState::lastPitchBendValue() const {
  return lastPitchBendValue_;
}

bool AppState::hasPitchBend() const {
  return hasPitchBend_;
}

uint32_t AppState::lastMidiActivityAtMs() const {
  return lastMidiActivityAtMs_;
}

size_t AppState::activeNoteCount() const {
  return activeNotes_.count();
}

ActiveNote AppState::activeNoteAt(size_t index) const {
  return activeNotes_.noteAt(index);
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

void AppState::clearActiveNotes() {
  if (activeNotes_.count() == 0) {
    return;
  }

  activeNotes_.clear();
  midiActivityRefreshNeeded_ = true;
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
  if (kind == MidiActivityKind::NoteOn && velocity == 0) {
    kind = MidiActivityKind::NoteOff;
  }

  receivedMidiMessageCount_ += 1;
  lastMidiActivityKind_ = kind;
  lastMidiChannel_ = channel;
  lastMidiNote_ = note;
  lastMidiVelocity_ = velocity;
  lastMidiActivityAtMs_ = activityAtMs;

  if (kind == MidiActivityKind::ActiveSensing) {
    activeSensingMessageCount_ += 1;
  } else if (kind == MidiActivityKind::NoteOn) {
    if (!activeNotes_.contains(channel, note) &&
        activeNotes_.count() >= ActiveNotes::MAX_ACTIVE_NOTES) {
      droppedActiveNoteCount_ += 1;
    } else {
      activeNotes_.noteOn(channel, note);
    }
  } else if (kind == MidiActivityKind::NoteOff) {
    activeNotes_.noteOff(channel, note);
  }

  midiActivityRefreshNeeded_ = true;
}

void AppState::recordNoteEvent(const NoteEvent& event, uint32_t activityAtMs) {
  const MidiActivityKind kind =
      event.type == NoteEventType::NoteOn ? MidiActivityKind::NoteOn
                                          : MidiActivityKind::NoteOff;
  recordMidiActivity(
      kind,
      event.channel,
      event.note,
      event.velocity,
      activityAtMs);
}

void AppState::recordControlChange(
    uint8_t channel,
    uint8_t controllerNumber,
    uint8_t controllerValue,
    uint32_t activityAtMs) {
  constexpr uint8_t SUSTAIN_CONTROLLER_NUMBER = 64;
  constexpr uint8_t SWITCH_ON_THRESHOLD = 64;

  receivedMidiMessageCount_ += 1;
  lastMidiActivityKind_ = MidiActivityKind::ControlChange;
  lastControlChangeChannel_ = channel;
  lastControlChangeNumber_ = controllerNumber;
  lastControlChangeValue_ = controllerValue;
  hasControlChange_ = true;
  lastMidiActivityAtMs_ = activityAtMs;

  if (controllerNumber == SUSTAIN_CONTROLLER_NUMBER) {
    sustainEnabled_ = controllerValue >= SWITCH_ON_THRESHOLD;
  }

  midiActivityRefreshNeeded_ = true;
}

void AppState::recordPitchBend(uint8_t channel, int bendValue, uint32_t activityAtMs) {
  receivedMidiMessageCount_ += 1;
  lastMidiActivityKind_ = MidiActivityKind::PitchBend;
  lastPitchBendChannel_ = channel;
  lastPitchBendValue_ = bendValue;
  hasPitchBend_ = true;
  lastMidiActivityAtMs_ = activityAtMs;
  midiActivityRefreshNeeded_ = true;
}

void AppState::markDisplayRefreshed() {
  fullDisplayRefreshNeeded_ = false;
  uptimeRefreshNeeded_ = false;
  midiActivityRefreshNeeded_ = false;
}
