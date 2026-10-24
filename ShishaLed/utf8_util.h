// Reine UTF-8-Funktionen für den Spielernamen (06 §6)
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace utf8 {

// Prüft strikt (keine Überlangkodierung, keine Surrogate, ≤ U+10FFFF, keine
// abgeschnittenen Folgen). Bei Erfolg steht die Zahl der Codepoints in *codepoints.
bool validate(const char* s, size_t len, size_t* codepoints);

// true, wenn ein Codepoint im Bereich U+0000–U+001F oder U+007F vorkommt.
// Erwartet gültiges UTF-8 (Steuerzeichen sind immer ein einzelnes Byte).
bool hasControlChars(const char* s, size_t len);

// Entfernt führende und abschließende Leerzeichen (U+0020): liefert Start und Länge.
void trimSpaces(const char* s, size_t len, size_t* start, size_t* outLen);

enum class NameResult : uint8_t {
  Ok,          // übernommen
  Normalized,  // leer nach Trim → Standardname
  Invalid,     // ungültiges UTF-8, Steuerzeichen oder zu lang
};

// Validiert und normalisiert einen Namen nach 06 §6 und schreibt ihn
// NUL-terminiert nach out. Bei Invalid bleibt out unverändert.
NameResult normalizeName(const char* s, size_t len, size_t maxCodepoints, const char* fallback,
                         char* out, size_t outSize);

}  // namespace utf8
