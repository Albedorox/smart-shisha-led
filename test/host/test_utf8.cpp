#include <cstring>
#include <string>

#include "app_config.h"
#include "test.h"
#include "utf8_util.h"

using utf8::NameResult;

namespace {

NameResult norm(const std::string& s, std::string* out = nullptr) {
  char buf[app::NAME_BUF] = "unverändert";
  const NameResult r =
      utf8::normalizeName(s.data(), s.size(), app::NAME_MAX_CODEPOINTS, app::DEFAULT_NAME, buf, sizeof(buf));
  if (out) *out = buf;
  return r;
}

std::string repeat(const std::string& s, int n) {
  std::string r;
  for (int i = 0; i < n; i++) r += s;
  return r;
}

}  // namespace

TEST("T-FW-19 gültige Namen") {
  std::string out;
  CHECK(norm("Alex", &out) == NameResult::Ok);
  CHECK(out == "Alex");
  CHECK(norm("  Jörg Müßig  ", &out) == NameResult::Ok);
  CHECK(out == "Jörg Müßig");
  CHECK(norm("Ali 🔥", &out) == NameResult::Ok);
  CHECK(out == "Ali 🔥");
}

TEST("T-FW-19 Länge: 24 Codepoints ja, 25 nein") {
  CHECK(norm(repeat("a", 24)) == NameResult::Ok);
  CHECK(norm(repeat("a", 25)) == NameResult::Invalid);
  CHECK(norm(repeat("ä", 24)) == NameResult::Ok);
  CHECK(norm(repeat("ä", 25)) == NameResult::Invalid);
  std::string out;
  CHECK(norm(repeat("😀", 24), &out) == NameResult::Ok);  // 96 Byte
  CHECK_EQ(out.size(), 96);
  CHECK(norm(repeat("😀", 25)) == NameResult::Invalid);
  CHECK(norm("  " + repeat("a", 24) + "  ") == NameResult::Ok);  // Leerzeichen zählen nach Trim nicht
}

TEST("T-FW-19 leer oder nur Leerzeichen → Unbekannt") {
  std::string out;
  CHECK(norm("", &out) == NameResult::Normalized);
  CHECK(out == "Unbekannt");
  CHECK(norm("     ", &out) == NameResult::Normalized);
  CHECK(out == "Unbekannt");
  char buf[app::NAME_BUF];
  CHECK(utf8::normalizeName(nullptr, 5, 24, "Unbekannt", buf, sizeof(buf)) == NameResult::Normalized);
}

TEST("T-FW-19 ungültiges UTF-8") {
  const char* bad[] = {
      "\xC0\x80",          // Überlangkodierung NUL
      "\xC1\xBF",          // Überlangkodierung
      "\xE0\x80\x80",      // Überlangkodierung 3 Byte
      "\xF0\x80\x80\x80",  // Überlangkodierung 4 Byte
      "\xED\xA0\x80",      // Surrogat U+D800
      "\xED\xBF\xBF",      // Surrogat U+DFFF
      "\xF4\x90\x80\x80",  // > U+10FFFF
      "\xF5\x80\x80\x80",  // F5 nie gültig
      "\xFF",
      "\x80",              // Folgebyte ohne Start
      "\xC3",              // abgeschnitten
      "a\xE2\x82",         // abgeschnitten
      "\xC3\x28",          // falsches Folgebyte
  };
  for (const char* b : bad) {
    std::string out;
    CHECK(norm(b, &out) == NameResult::Invalid);
    CHECK(out == "unverändert");
  }
  CHECK(norm("\xEF\xBF\xBF") == NameResult::Ok);      // U+FFFF ist gültig
  CHECK(norm("\xF4\x8F\xBF\xBF") == NameResult::Ok);  // U+10FFFF
}

TEST("T-FW-19 Steuerzeichen") {
  CHECK(norm("Al\tex") == NameResult::Invalid);
  CHECK(norm("Alex\n") == NameResult::Invalid);
  CHECK(norm(std::string("Al\0ex", 5)) == NameResult::Invalid);
  CHECK(norm("Al\x7F" "ex") == NameResult::Invalid);
  CHECK(norm("\t") == NameResult::Invalid);
}

TEST("UTF-8 Codepoints zählen") {
  size_t n = 0;
  CHECK(utf8::validate("aä€😀", strlen("aä€😀"), &n));
  CHECK_EQ(n, 4);
}
