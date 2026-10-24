#include "utf8_util.h"

#include <string.h>

namespace utf8 {

bool validate(const char* s, size_t len, size_t* codepoints) {
  const uint8_t* p = (const uint8_t*)s;
  size_t i = 0, count = 0;
  while (i < len) {
    const uint8_t b0 = p[i];
    size_t n;
    uint32_t cp, min;
    if (b0 < 0x80) {
      n = 1; cp = b0; min = 0;
    } else if ((b0 & 0xE0) == 0xC0) {
      n = 2; cp = b0 & 0x1F; min = 0x80;
    } else if ((b0 & 0xF0) == 0xE0) {
      n = 3; cp = b0 & 0x0F; min = 0x800;
    } else if ((b0 & 0xF8) == 0xF0) {
      n = 4; cp = b0 & 0x07; min = 0x10000;
    } else {
      return false;  // Folgebyte am Anfang oder F8..FF
    }
    if (len - i < n) return false;
    for (size_t k = 1; k < n; k++) {
      const uint8_t b = p[i + k];
      if ((b & 0xC0) != 0x80) return false;
      cp = (cp << 6) | (b & 0x3F);
    }
    if (cp < min) return false;                       // Überlangkodierung
    if (cp >= 0xD800 && cp <= 0xDFFF) return false;   // Surrogate
    if (cp > 0x10FFFF) return false;
    i += n;
    count++;
  }
  if (codepoints) *codepoints = count;
  return true;
}

bool hasControlChars(const char* s, size_t len) {
  for (size_t i = 0; i < len; i++) {
    const uint8_t b = (uint8_t)s[i];
    if (b < 0x20 || b == 0x7F) return true;
  }
  return false;
}

void trimSpaces(const char* s, size_t len, size_t* start, size_t* outLen) {
  size_t a = 0, e = len;
  while (a < e && s[a] == ' ') a++;
  while (e > a && s[e - 1] == ' ') e--;
  *start = a;
  *outLen = e - a;
}

NameResult normalizeName(const char* s, size_t len, size_t maxCodepoints, const char* fallback,
                         char* out, size_t outSize) {
  if (!s) len = 0;
  size_t cps = 0;
  if (len && !validate(s, len, &cps)) return NameResult::Invalid;
  size_t a = 0, n = 0;
  if (len) trimSpaces(s, len, &a, &n);
  if (n && hasControlChars(s + a, n)) return NameResult::Invalid;
  if (n == 0) {
    const size_t fl = strlen(fallback);
    if (fl + 1 > outSize) return NameResult::Invalid;
    memcpy(out, fallback, fl + 1);
    return NameResult::Normalized;
  }
  validate(s + a, n, &cps);  // Codepoints nach dem Trim
  if (cps > maxCodepoints || n + 1 > outSize) return NameResult::Invalid;
  memcpy(out, s + a, n);
  out[n] = '\0';
  return NameResult::Ok;
}

}  // namespace utf8
