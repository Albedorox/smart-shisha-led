#include "test.h"

int main() {
  int failedTests = 0;
  for (const TestCase& t : testRegistry()) {
    const int before = testFailures();
    t.fn();
    const bool ok = testFailures() == before;
    if (!ok) failedTests++;
    std::printf("%s %s\n", ok ? "  ok  " : "FEHLER", t.name);
  }
  std::printf("\n%zu Tests, %d fehlgeschlagen\n", testRegistry().size(), failedTests);
  return failedTests ? 1 : 0;
}
