// Spielername, Rangliste (Top 5, nur RAM), Zugzähler, letzter Zug (01 §6, 06 §6–7)
#pragma once

#include <stddef.h>
#include <stdint.h>

#include "app_config.h"
#include "pull_detector.h"
#include "utf8_util.h"

struct LeaderboardEntry {
  char     name[app::NAME_BUF] = {};
  uint32_t durationMs = 0;
  uint64_t endedAtMs  = 0;
  uint32_t seq        = 0;   // Gleichstand: kleinere seq = früher beendet
  bool     truncated  = false;
  bool     diagnostic = false;
};

class SessionManager {
 public:
  SessionManager();

  // Name gilt ab dem nächsten Zugbeginn (SES-05)
  utf8::NameResult setName(const char* utf8Name, size_t len);
  const char* currentName() const { return currentName_; }

  void onPullStarted(uint32_t seq);
  void onPullCompleted(const PullRecord& rec, bool diagnostic);

  // Leert die Rangliste, setzt pullCount auf 0 (D-11); lastPull und laufender Zug bleiben.
  void resetLeaderboard();

  uint8_t entryCount() const { return entryCount_; }
  const LeaderboardEntry& entry(uint8_t i) const { return entries_[i]; }
  uint32_t pullCount() const { return pullCount_; }
  bool hasLastPull() const { return hasLastPull_; }
  const LeaderboardEntry& lastPull() const { return lastPull_; }
  uint32_t lbVersion() const { return lbVersion_; }

 private:
  void insert(const LeaderboardEntry& e);

  char             currentName_[app::NAME_BUF];
  char             pullName_[app::NAME_BUF];
  uint32_t         pullSeq_    = 0;
  bool             hasPullName_ = false;
  LeaderboardEntry entries_[app::LEADERBOARD_SIZE];
  uint8_t          entryCount_ = 0;
  uint32_t         pullCount_  = 0;
  bool             hasLastPull_ = false;
  LeaderboardEntry lastPull_;
  uint32_t         lbVersion_  = 0;
};
