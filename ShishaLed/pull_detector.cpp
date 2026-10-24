#include "pull_detector.h"

#include "time_util.h"

void PullDetector::evaluate(uint32_t now, uint64_t uptimeMs, bool impulse, bool levelActive,
                            uint32_t tailMs, uint32_t maxPullMs) {
  // Dauer-aktiv-Erkennung (04 §4.3)
  if (levelActive) {
    if (!stuckRun_) {
      stuckRun_ = true;
      activeSince_ = now;
    }
    if (elapsed(now, activeSince_) >= app::STUCK_ACTIVE_MS) fault_ = SensorFault::StuckActive;
  } else {
    if (stuckRun_) inactiveSince_ = now;
    stuckRun_ = false;
    if (fault_ == SensorFault::StuckActive && elapsed(now, inactiveSince_) >= app::STUCK_CLEAR_MS) {
      fault_ = SensorFault::None;
    }
  }
  levelActive_ = levelActive;

  if (impulse) {
    hasPulse_ = true;
    lastPulseAny_ = now;
  }

  if (!state_.active) {
    if (waitQuiet_) {
      if (impulse) state_.lastPulseMs = now;
      else if (elapsed(now, state_.lastPulseMs) >= tailMs) waitQuiet_ = false;
      return;
    }
    if (impulse) {
      state_.active = true;
      state_.startMs = state_.lastPulseMs = now;
      state_.seq++;
      startPending_ = true;
    }
    return;
  }

  if (impulse) state_.lastPulseMs = now;
  if (elapsed(now, state_.lastPulseMs) >= tailMs) {
    finish(now, uptimeMs, false);
  } else if (elapsed(now, state_.startMs) >= maxPullMs) {
    finish(now, uptimeMs, true);
  }
}

void PullDetector::finish(uint32_t now, uint64_t uptimeMs, bool truncated) {
  PullRecord rec;
  rec.seq = state_.seq;
  rec.durationMs = elapsed(now, state_.startMs);
  rec.endedAtMs = uptimeMs;
  rec.truncated = truncated;
  if (qCount_ == QUEUE) {  // ältesten verwerfen; tritt bei Abholung je Schleife nicht auf
    qHead_ = (qHead_ + 1) % QUEUE;
    qCount_--;
  }
  queue_[(qHead_ + qCount_) % QUEUE] = rec;
  qCount_++;
  state_.active = false;
  waitQuiet_ = truncated;
}

bool PullDetector::takeStartedPull(uint32_t* seq) {
  if (!startPending_) return false;
  startPending_ = false;
  if (seq) *seq = state_.seq;
  return true;
}

bool PullDetector::takeCompletedPull(PullRecord* rec) {
  if (qCount_ == 0) return false;
  *rec = queue_[qHead_];
  qHead_ = (qHead_ + 1) % QUEUE;
  qCount_--;
  return true;
}

void PullDetector::addEdges(uint32_t count, uint64_t uptimeMs) {
  if (!count) return;
  const uint64_t sec = uptimeMs / 1000;
  const uint8_t i = sec % app::DIAG_WINDOW_S;
  if (bucketSecond_[i] != sec) {
    bucketSecond_[i] = sec;
    bucketCount_[i] = 0;
  }
  bucketCount_[i] += count;
}

SensorDiag PullDetector::diag(uint32_t now, uint64_t uptimeMs) const {
  const uint64_t sec = uptimeMs / 1000;
  uint32_t sum = 0;
  for (uint8_t i = 0; i < app::DIAG_WINDOW_S; i++) {
    if (bucketCount_[i] && bucketSecond_[i] + app::DIAG_WINDOW_S > sec) sum += bucketCount_[i];
  }
  SensorDiag d;
  d.levelActive = levelActive_;
  d.hasPulse = hasPulse_;
  d.lastPulseAgoMs = hasPulse_ ? elapsed(now, lastPulseAny_) : 0;
  d.pulsesInWindow = sum > 0xFFFF ? 0xFFFF : (uint16_t)sum;
  d.fault = fault_;
  return d;
}
