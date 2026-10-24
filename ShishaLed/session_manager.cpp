#include "session_manager.h"

#include <string.h>

namespace {

void copyName(char* dst, const char* src) {
  strncpy(dst, src, app::NAME_BUF - 1);
  dst[app::NAME_BUF - 1] = '\0';
}

}  // namespace

SessionManager::SessionManager() {
  copyName(currentName_, app::DEFAULT_NAME);
  copyName(pullName_, app::DEFAULT_NAME);
}

utf8::NameResult SessionManager::setName(const char* utf8Name, size_t len) {
  return utf8::normalizeName(utf8Name, len, app::NAME_MAX_CODEPOINTS, app::DEFAULT_NAME, currentName_,
                             sizeof(currentName_));
}

void SessionManager::onPullStarted(uint32_t seq) {
  copyName(pullName_, currentName_);
  pullSeq_ = seq;
  hasPullName_ = true;
}

void SessionManager::onPullCompleted(const PullRecord& rec, bool diagnostic) {
  LeaderboardEntry e;
  copyName(e.name, hasPullName_ && pullSeq_ == rec.seq ? pullName_ : currentName_);
  e.durationMs = rec.durationMs;
  e.endedAtMs = rec.endedAtMs;
  e.seq = rec.seq;
  e.truncated = rec.truncated;
  e.diagnostic = diagnostic;

  lastPull_ = e;
  hasLastPull_ = true;
  if (diagnostic) return;  // SEN-10: weder Rangliste noch Zähler

  pullCount_++;
  insert(e);
  lbVersion_++;
}

void SessionManager::insert(const LeaderboardEntry& e) {
  // Absteigend nach Dauer; bei Gleichstand bleibt der früher beendete Zug vorn (SES-02).
  uint8_t pos = 0;
  while (pos < entryCount_ && entries_[pos].durationMs >= e.durationMs) pos++;
  if (pos >= app::LEADERBOARD_SIZE) return;
  const uint8_t last = entryCount_ < app::LEADERBOARD_SIZE ? entryCount_ : app::LEADERBOARD_SIZE - 1;
  for (uint8_t i = last; i > pos; i--) entries_[i] = entries_[i - 1];
  entries_[pos] = e;
  if (entryCount_ < app::LEADERBOARD_SIZE) entryCount_++;
}

void SessionManager::resetLeaderboard() {
  entryCount_ = 0;
  pullCount_ = 0;
  lbVersion_++;
}
