// Minimaler Testrahmen für die Host-Tests (ohne externe Abhängigkeiten)
#pragma once

#include <cstdio>
#include <cstdlib>
#include <functional>
#include <vector>

struct TestCase {
  const char* name;
  std::function<void()> fn;
};

inline std::vector<TestCase>& testRegistry() {
  static std::vector<TestCase> r;
  return r;
}

inline int& testFailures() {
  static int n = 0;
  return n;
}

struct TestRegistrar {
  TestRegistrar(const char* name, std::function<void()> fn) { testRegistry().push_back({name, fn}); }
};

#define TEST_CAT2(a, b) a##b
#define TEST_CAT(a, b) TEST_CAT2(a, b)
#define TEST(name)                                                           \
  static void TEST_CAT(test_fn_, __LINE__)();                                \
  static TestRegistrar TEST_CAT(test_reg_, __LINE__)(name, TEST_CAT(test_fn_, __LINE__)); \
  static void TEST_CAT(test_fn_, __LINE__)()

#define CHECK(cond)                                                          \
  do {                                                                       \
    if (!(cond)) {                                                           \
      std::printf("    FEHLER %s:%d: %s\n", __FILE__, __LINE__, #cond);      \
      testFailures()++;                                                      \
    }                                                                        \
  } while (0)

#define CHECK_EQ(a, b)                                                       \
  do {                                                                       \
    const long long va_ = (long long)(a), vb_ = (long long)(b);              \
    if (va_ != vb_) {                                                        \
      std::printf("    FEHLER %s:%d: %s == %s (%lld != %lld)\n", __FILE__, __LINE__, #a, #b, va_, vb_); \
      testFailures()++;                                                      \
    }                                                                        \
  } while (0)

#define CHECK_NEAR(a, b, tol)                                                \
  do {                                                                       \
    const long long va_ = (long long)(a), vb_ = (long long)(b);              \
    if (va_ - vb_ > (tol) || vb_ - va_ > (tol)) {                            \
      std::printf("    FEHLER %s:%d: %s ≈ %s (%lld vs %lld, Toleranz %d)\n", __FILE__, __LINE__, #a, #b, \
                  va_, vb_, (int)(tol));                                     \
      testFailures()++;                                                      \
    }                                                                        \
  } while (0)
