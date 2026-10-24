# 04 – Firmware-Architektur: Module, Zustandsmaschinen, Datenmodelle

Stand: 2026-10-23 · Phase A · Dieses Dokument enthält **keinen produktiven Code**. Pseudocode dient nur der eindeutigen Spezifikation.

## 1. Werkzeugkette und Bibliotheken

| Thema | Festlegung (Vorschlag) | Status |
|---|---|---|
| IDE | Arduino IDE 2.x | Lastenheft |
| Board-Paket | „esp32 by Espressif Systems“ 3.x (exakte Version wird zu Beginn von B1 festgelegt und in `README` notiert) | Vorschlag |
| Board-Einstellung | „ESP32C3 Dev Module“, *USB CDC On Boot: Enabled*, Flash 4 MB, Partition Default | Vorschlag, M-01 |
| Webserver | eingebaute Bibliothek `WebServer` des Board-Pakets (synchron, in `loop()` bedient), **keine** externe Async-Bibliothek | D-17 |
| LED-Treiber | Adafruit NeoPixel (RMT-basiert auf ESP32) | D-17 |
| JSON | Ausgabe per `snprintf` in statischen Puffer; Eingabe (nur POST) mit ArduinoJson 7 und Größenlimit | D-18 |
| Weboberfläche | eine HTML-Datei mit eingebettetem CSS/JS, gzip-komprimiert als `const uint8_t[]` im Flash (`web_assets.h`, per Skript erzeugt) | 05 §1 |

Externe Bibliotheken werden mit exakter Version in der Stückliste der Software geführt.

## 2. Modulstruktur

### 2.1 Namenskonflikte mit dem Lastenheft

| Lastenheft | Problem | Vorschlag (D-19) |
|---|---|---|
| `main.cpp` | Die Arduino IDE verlangt eine `.ino`-Datei mit dem Namen des Sketch-Ordners als Einstiegspunkt | `ShishaLed/ShishaLed.ino` übernimmt die Rolle von `main.cpp` und enthält nur `setup()`/`loop()` und die Verdrahtung der Module |
| Modul `WebServer` | kollidiert mit der Klasse `WebServer` des Board-Pakets | Modul heißt `WebInterface` (`web_interface.h/.cpp`) |

### 2.2 Dateien

```
ShishaLed/
├── ShishaLed.ino          Einstieg: setup(), loop(), Reihenfolge der Updates
├── app_config.h           Kompilierkonstanten: Pins, Polarität, Grenzen, Standardwerte, Version
├── time_util.h            elapsed(now, since) mit uint32-Überlaufsicherheit, Uptime 64 bit
├── config_manager.h/.cpp  RAM-Konfiguration, Validierung, Reset, configVersion
├── sensor_manager.h/.cpp  ISR-Latch, 10-ms-Auswertung, Zug-Zustandsmaschine, Diagnosezähler
├── session_manager.h/.cpp Spielername, Rangliste, pullCount, lastPull, lbVersion, Diagnoseflag
├── led_controller.h/.cpp  Moduslogik, Farbverlauf, Fade-out, Stromgrenze, Ausgabe
├── color_math.h/.cpp      reine Funktionen: Interpolation, Skalierung, Stromabschätzung
├── utf8_util.h/.cpp       reine Funktionen: UTF-8-Validierung, Codepoint-Zählung, Trim
├── web_interface.h/.cpp   AP-Start, Routen, JSON-Aufbau, Fehlerantworten
└── web_assets.h           generiert: gzip-Daten der Weboberfläche
tools/
└── build_web_assets.py    erzeugt web_assets.h aus web/index.html (nur Build-Hilfe)
web/
└── index.html             Quelle der Weboberfläche (Phase B)
test/host/                 Host-Tests (g++) für color_math, utf8_util, Zustandsmaschine, Rangliste
```

**Testbarkeit:** `color_math`, `utf8_util` sowie die Kernlogik von `SensorManager` und `SessionManager` hängen nicht von Arduino-Headern ab (Zeit wird als Parameter `now` übergeben, Pinzugriff über eine kleine Schnittstelle). Dadurch laufen sie auf dem PC in Host-Tests (09 T-FW-xx).

### 2.3 Zuständigkeiten und Schnittstellen

| Modul | Verantwortung | Öffentliche Schnittstelle (Auszug) | Abhängigkeiten |
|---|---|---|---|
| ConfigManager | hält `Config`, validiert und wendet Änderungen alles-oder-nichts an, zählt `configVersion` | `const Config& get()`, `ApplyResult apply(const ConfigPatch&)`, `void reset(ResetScope)`, `uint32_t version()` | – |
| SensorManager | liest Sensor, erkennt Impulse und Züge, liefert abgeschlossene Züge genau einmal, Diagnosedaten | `begin()`, `update(now, tailMs)`, `PullState state()`, `bool takeCompletedPull(PullRecord&)`, `SensorDiag diag(now)` | time_util |
| SessionManager | Spielername, Übernahme bei Zugbeginn, Rangliste, Zähler | `setName(const char*)`, `onPullStarted(seq)`, `onPullCompleted(const PullRecord&, bool diagnostic)`, `resetLeaderboard()`, Getter | utf8_util |
| LedController | berechnet Soll-Farbe aus Modus, Zugzustand, Fade; wendet Helligkeit und Stromgrenze an; schreibt LEDs | `begin()`, `update(now, const Config&, const PullState&)`, `LedView view()` | color_math, NeoPixel |
| WebInterface | WLAN-AP, HTTP-Routen, Request-Validierung, JSON | `begin()`, `handle()` | alle obigen (nur lesend bzw. über deren Schnittstellen) |

Kopplungsregel: SensorManager und LedController kennen den Webserver nicht. WebInterface ändert Zustand ausschließlich über `ConfigManager::apply/reset`, `SessionManager::setName/resetLeaderboard`.

## 3. Datenmodelle (RAM)

```
enum class Mode : uint8_t { Smart, Steady, Off };
enum class Phase : uint8_t { Idle, Pull, Fade };          // Anzeigezustand
enum class SensorFault : uint8_t { None, StuckActive };

struct Rgb { uint8_t r, g, b; };

struct Config {                 // Standardwerte siehe 01 §9
  Mode     mode        = Smart;
  uint16_t yellowMs    = 3000;
  uint16_t redMs       = 6000;
  uint16_t tailMs      = 1000;
  uint8_t  brightness  = 64;
  Rgb      steadyColor = {255,160,60};   // offen: D-07
  bool     diagnostic  = false;
};

struct PullState {
  bool     active;
  uint32_t startMs;            // millis() des ersten Impulses
  uint32_t lastPulseMs;
  uint32_t seq;                // laufende Zugnummer seit Start
};

struct PullRecord {            // abgeschlossener Zug
  uint32_t seq;
  uint32_t durationMs;
  uint64_t endedAtMs;          // Uptime 64 bit
  bool     truncated;          // durch maxPullMs beendet
};

struct LeaderboardEntry {
  char     name[97];           // max. 24 Codepoints × 4 Byte + NUL
  uint32_t durationMs;
  uint64_t endedAtMs;
  uint32_t seq;                // Tie-Break: kleinere seq = früher beendet
};

struct Session {
  char     currentName[97] = "Unbekannt";
  char     pullName[97];       // beim Zugbeginn übernommen
  LeaderboardEntry entries[5];
  uint8_t  entryCount;
  uint32_t pullCount;
  bool     hasLastPull;
  LeaderboardEntry lastPull;
  uint32_t lbVersion;          // +1 bei jeder Änderung der Rangliste oder des Zählers
};

struct SensorDiag {
  bool     levelActive;        // aktueller Pegel
  bool     hasPulse;           // seit Start mindestens ein Impuls
  uint32_t lastPulseAgoMs;
  uint16_t pulsesInWindow;     // Impulsflanken in den letzten 10 s
  SensorFault fault;
};
```

Speicherbedarf (Schätzung): Session ≈ 0,8 KB, Konfiguration < 32 Byte, JSON-Ausgabepuffer 1,5 KB statisch, Weboberfläche 15–30 KB im Flash.

Alle Werte werden beim Start auf die Standardwerte gesetzt. Es gibt keinen Lese- oder Schreibzugriff auf Flash/NVS/Dateisystem für diese Daten.

## 4. Sensor und Zug-Zustandsmaschine

### 4.1 Impulserkennung (D-12)

- Eine Interrupt-Routine auf der aktiven Flanke (bei `SENSOR_ACTIVE_LOW` fallend) setzt ein Latch-Flag und erhöht einen Flankenzähler. Sie macht sonst nichts.
- Alle 10 ms (gemessen mit `millis()`) wird ausgewertet:
  `impuls = (Pegel ist aktiv) ODER (Latch gesetzt)`; danach wird das Latch atomar zurückgesetzt.
- Hat sich die Schleife verspätet (> 10 ms), wird **eine** Auswertung durchgeführt und der nächste Termin auf `now + 10` gesetzt (kein Nachholen mehrerer Zyklen). Das Latch stellt sicher, dass kurze Impulse dazwischen nicht verloren gehen.
- Begründung: SW-420-Impulse können kürzer als 10 ms sein; ein reines Abtasten des Pegels würde sie bei Schleifenverzögerungen (z. B. während einer HTTP-Antwort) verpassen.

### 4.2 Zustände und Übergänge

```
            impuls                         impuls (lastPulse = now)
   ┌──────┐ ─────────► ┌────────┐ ◄───────────────┐
   │ IDLE │            │ ACTIVE │ ────────────────┘
   └──────┘ ◄───────── └────────┘
        elapsed(now, lastPulse) ≥ tailMs   ODER   elapsed(now, start) ≥ maxPullMs
        → PullRecord erzeugen (genau einmal), seq++
```

| Übergang | Bedingung | Aktionen |
|---|---|---|
| IDLE → ACTIVE | Auswertung mit `impuls` | `start = lastPulse = now`; `seq++`; Ereignis *PullStarted(seq)* → SessionManager kopiert `currentName` nach `pullName` |
| ACTIVE → ACTIVE | Auswertung mit `impuls` | `lastPulse = now` |
| ACTIVE → IDLE | `elapsed(now, lastPulse) ≥ tailMs` | `durationMs = elapsed(now, start)`; Record in Ausgangspuffer (Ringpuffer 4 Einträge); `truncated = false` |
| ACTIVE → IDLE | `elapsed(now, start) ≥ maxPullMs` (D-08, Vorschlag 60.000) | wie oben, `truncated = true`; der nächste Zug kann erst starten, nachdem der Sensor mindestens `tailMs` inaktiv war (verhindert Endlosschleife bei Dauersignal) |

- `tailMs`-Änderungen gelten sofort, auch für einen laufenden Zug.
- Zeitvergleich ausschließlich als `(uint32_t)(now - t) ≥ x` → korrekt über den `millis()`-Überlauf (≈ 49,7 Tage) hinweg.
- *Genau einmal:* `takeCompletedPull()` entnimmt den Record aus dem Puffer. Er wird in derselben `loop()`-Iteration vom SessionManager verarbeitet. Da HTTP-Handler im selben Thread laufen, gibt es keine Nebenläufigkeit außer dem ISR-Latch.
- Mit `truncated = true` beendete Züge werden in die Rangliste übernommen und in der UI mit „max.“ gekennzeichnet (D-08).

### 4.3 Fehlerzustände des Sensors

| Zustand | Erkennung | Anzeige |
|---|---|---|
| Dauerhaft aktiv | Pegel ununterbrochen aktiv ≥ 5.000 ms | `fault = "stuck_active"`, Hinweis im Sensor-Screen; wird zurückgesetzt, sobald der Pegel 1 s inaktiv ist |
| Kein Signal seit Start | `hasPulse = false` | nur Hinweis („Seit dem Start wurde keine Erschütterung erkannt – Empfindlichkeit/Position prüfen“), kein Fehler, da Ruhe der Normalfall ist |

Ein automatischer Hardware-Selbsttest wird nicht behauptet, da der Sensor nur einen digitalen Ausgang hat.

### 4.4 Diagnosedaten

- `pulsesInWindow`: Zahl der Flanken (aus dem ISR-Zähler) in den letzten 10 s, ermittelt über 10 Eimer à 1 s.
- `lastPulseAgoMs`: Zeit seit dem letzten Impuls (nur gültig, wenn `hasPulse`).
- Diagnosemodus (`config.diagnostic`): Zugerkennung und LED-Verhalten laufen normal; `onPullCompleted` verwirft den Record für Rangliste und `pullCount`, aktualisiert aber `lastPull` mit Markierung `diagnostic = true`. Der Modus endet bei Konfigurations-Reset oder Neustart.

## 5. Hauptschleife

```
setup():
  Serial (Debug, optional), ConfigManager.reset(All), SessionManager.init()
  LedController.begin() → alle LEDs aus
  SensorManager.begin() → Pin-Modus, ISR anmelden
  WebInterface.begin() → softAP, Routen, server.begin()

loop():
  now = millis()
  sensor.update(now, config.tailMs)
  if sensor hat PullStarted:   session.onPullStarted(seq)
  while sensor.takeCompletedPull(rec): session.onPullCompleted(rec, config.diagnostic)
  led.update(now, config, sensor.state())
  web.handle()        // bedient höchstens eine Anfrage pro Durchlauf
```

Zeitbudget: Ein HTTP-Request (Status-JSON ≈ 0,8 KB) muss in < 10 ms bearbeitet sein; die Auslieferung der Weboberfläche (≈ 25 KB) darf länger dauern, Impulse gehen dank Latch nicht verloren. Ziel der maximalen Schleifendauer: < 20 ms, harte Grenze 50 ms (T-LONG-01).

## 6. LED-Logik

### 6.1 Soll-Farbe je Modus und Phase

| Modus | Zug aktiv | Fade läuft | Ausgabe | Phase (API) |
|---|---|---|---|---|
| Smart | ja | – | `gradient(elapsed(now, start))` | `pull` |
| Smart | nein | ja (`elapsed(now, pullEnd) < 1000`) | `lastPullColor × fadeFactor` | `fade` |
| Smart | nein | nein | aus | `idle` |
| Steady | egal | – | `steadyColor` | `pull` wenn Zug aktiv, sonst `idle` |
| Off | egal | – | aus | `pull` wenn Zug aktiv, sonst `idle` |

- Wechsel zu Smart während eines aktiven Zuges zeigt sofort die zur bisherigen Dauer passende Farbe.
- Wechsel weg von Smart während des Fade-outs bricht den Fade-out ab.
- Ein neuer Zug während des Fade-outs: Phase `pull`, Farbe beginnt bei Grün (Zugdauer 0).

### 6.2 Farbverlauf (verbindlich)

Mit `t` = Zugdauer in ms, `Y` = `yellowMs`, `R` = `redMs`, `lerp(a, b, n, d) = a + ((b − a)·n + d/2) / d` in Ganzzahlarithmetik (kaufmännisch gerundet):

| Bereich | R-Kanal | G-Kanal | B |
|---|---|---|---|
| `t ≤ 0` | 0 | 255 | 0 |
| `0 < t < Y` | `lerp(0, 255, t, Y)` | 255 | 0 |
| `Y ≤ t < R` | 255 | `lerp(255, 0, t − Y, R − Y)` | 0 |
| `t ≥ R` | 255 | 0 | 0 |

Referenzwerte bei Standardwerten (Testgrundlage T-FW-10):

| t | 0 s | 1,5 s | 3 s | 4,5 s | 6 s | 10 s |
|---|---|---|---|---|---|---|
| RGB | (0,255,0) | (128,255,0) | (255,255,0) | (255,128,0)* | (255,0,0) | (255,0,0) |

\* `255 + ((0 − 255)·1500 + 1500)/3000` = 255 − 127 = 128 (Ganzzahldivision rundet gegen null). Die Testfälle verwenden die so berechneten Werte mit Toleranz ± 1.

Die Werte sind logische Farben vor der Helligkeitsskalierung. Eine Gammakorrektur ist in V1 nicht vorgesehen (D-20, optional).

### 6.3 Fade-out

`fadeFactor = 255 − (255 · elapsed(now, pullEnd) + 500) / 1000` für `elapsed < 1000`, danach 0. `lastPullColor` ist die letzte im Zug berechnete Verlaufsfarbe. Dauer fest 1.000 ms (D-10).

### 6.4 Helligkeit und Stromgrenze

1. `B = config.brightness` (0–255).
2. Stromschätzung für `LED_COUNT` = 24 gleichfarbige LEDs: `I_mA = 24 · 20 · (r + g + b) / 255 · B / 255` (`LED_MA_PER_CHANNEL = 20`, Annahme, nach M-05 anpassen).
3. Ist `I_mA > LED_CURRENT_LIMIT_MA` (Vorschlag 500, D-05): `B_eff = B · LIMIT / I_mA`, sonst `B_eff = B`.
4. Ausgabe je Kanal `c · B_eff / 255`.
5. `limited = (B_eff < B)` wird in der API gemeldet.

### 6.5 Ausgabe

- Neuberechnung in jedem Durchlauf; Übertragung an die LEDs nur bei geänderten Werten und höchstens alle 20 ms (50 Hz). Übertragungsdauer 24 LEDs ≈ 0,75 ms.
- Optional (D-29/CR-02): Keep-Alive für Powerbanks als Teil des LedController (Lichtpuls) bzw. eigenes Mini-Modul `keep_alive` (Pulslast an GPIO 5); Periode und Pulsdauer als Konstanten in `app_config.h`; nicht blockierend über `millis()`.
- Optional (D-30): Bereitschaftsglimmen im Smart-Ruhezustand (z. B. eine LED mit Helligkeit 2), abschaltbar.
- Beim Start werden die LEDs explizit gelöscht (WS2812B können nach dem Einschalten zufällige Farben zeigen).

## 7. WLAN und Webserver

| Einstellung | Wert |
|---|---|
| Modus | Soft-AP, kein Station-Modus |
| SSID / Passwort | `Shisha-LED-Control` / offen |
| IP / Maske / Gateway | 192.168.4.1 / 255.255.255.0 / 192.168.4.1 |
| Kanal | 1 (Vorschlag; bei Störungen 6 oder 11) |
| Max. Clients | 4 |
| Sendeleistung | Konstante `WIFI_TX_POWER`, Start 8,5 dBm wegen bekannter SuperMini-Antennenprobleme (R-07); in T-HW-09 optimieren |
| Modem-Sleep | aus (bessere Antwortzeiten) |
| Captive Portal / DNS | nicht in V1 (D-21) |
| HTTP | Port 80, Routen gemäß 06; `Content-Encoding: gzip` für `/` |

## 8. Robustheit

| Thema | Maßnahme |
|---|---|
| Dynamischer Speicher | Keine Allokation in Sensor- und LED-Pfad. Status-JSON in statischen Puffer. Die eingebaute `WebServer`-Bibliothek nutzt intern `String`; das ist pro Request begrenzt und wird im Langzeittest über den minimalen freien Heap überwacht (T-LONG-01). |
| Request-Größe | `Content-Length` > 512 → 413 ohne Verarbeitung; JSON-Parser mit festem Dokumentlimit |
| Arraygrenzen | Rangliste fest 5, Namen fest 97 Byte, alle Kopien mit Längenprüfung |
| Zu lange Namen | Validierung vor dem Kopieren; Ablehnung statt Abschneiden |
| `millis()`-Überlauf | nur Differenzen; Uptime 64 bit über Überlaufzähler |
| Verbindungsabbrüche | serverseitig zustandslos; Client erkennt Ausfall per Timeout (05 §3) |
| Watchdog | Standard-Task-Watchdog bleibt aktiv; keine Schleife > 50 ms |
| Debug | Serielle Logs über USB-CDC mit Kompilierschalter `LOG_LEVEL`; im Auslieferungszustand nur Fehler |

## 9. Versionierung

`FW_VERSION` in `app_config.h` nach SemVer, Start `1.0.0`. Build-Kennung (Datum) wird in `/api/status` mitgeliefert.
