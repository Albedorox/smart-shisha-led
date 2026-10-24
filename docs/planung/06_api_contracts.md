# 06 – HTTP-API und JSON-Datenverträge

Stand: 2026-10-23 · Phase A · Alle Beispielwerte in diesem Dokument sind **Beispiele zur Formatbeschreibung**, keine Messwerte. Die Firmware liefert ausschließlich ihren tatsächlichen Zustand.

## 1. Allgemeine Regeln

| Regel | Festlegung |
|---|---|
| Basis | `http://192.168.4.1`, Port 80 |
| Format | JSON, UTF-8, `Content-Type: application/json; charset=utf-8` |
| Request-Body | nur bei POST; `Content-Type: application/json` erforderlich; max. 512 Byte |
| Zahlen | Ganzzahlen; Zeiten in ms; Helligkeit/Farbkanäle 0–255 |
| Unbekannte Felder | werden abgelehnt (400 `unknown_field`) – verhindert stille Tippfehler |
| Validierung | alles oder nichts: bei einem Fehler wird nichts übernommen |
| Caching | API: `Cache-Control: no-store`; `/`: `Cache-Control: no-cache` + `ETag` = Firmware-Build |
| CORS | keine CORS-Header (nur Same-Origin) |
| Authentifizierung | keine (offener AP, siehe 01 SAF-01) |
| Gleichzeitigkeit | Der Server bearbeitet Anfragen nacheinander in der Hauptschleife. Jede Anfrage ist atomar. Bei konkurrierenden Änderungen gilt *last write wins*; `configVersion` und `lbVersion` machen Änderungen anderer Clients sichtbar. Wartende Verbindungen puffert der TCP-Stack; bei Überlast antwortet der Server ggf. verspätet, nie mit Teilzuständen. |

### 1.1 Fehlerformat (alle Fehler)

```json
{
  "ok": false,
  "error": {
    "code": "validation_failed",
    "field": "redMs",
    "message": "Die Zeit bis Rot muss größer als die Zeit bis Gelb sein."
  }
}
```

`field` ist optional. `message` ist deutscher Klartext für die direkte Anzeige.

### 1.2 Fehlercodes

| HTTP | `code` | Bedeutung |
|---|---|---|
| 400 | `invalid_json` | Body ist kein gültiges JSON oder kein Objekt |
| 400 | `unknown_field` | Feld nicht vorgesehen |
| 400 | `invalid_type` | falscher Datentyp |
| 400 | `out_of_range` | Wert außerhalb des Bereichs |
| 400 | `validation_failed` | Kombination ungültig (z. B. Rot ≤ Gelb) |
| 400 | `invalid_name` | Name zu lang, ungültiges UTF-8 oder Steuerzeichen |
| 400 | `confirmation_required` | Reset ohne `"confirm": true` |
| 400 | `empty_patch` | keine änderbaren Felder übergeben |
| 404 | `not_found` | Pfad unbekannt |
| 405 | `method_not_allowed` | falsche Methode (Header `Allow` gesetzt) |
| 413 | `payload_too_large` | Body > 512 Byte |
| 415 | `unsupported_media_type` | kein `application/json` |
| 500 | `internal_error` | unerwarteter Zustand (soll nie auftreten; wird geloggt) |

## 2. `GET /`

Liefert die Weboberfläche (eine Datei, gzip). Header: `Content-Type: text/html; charset=utf-8`, `Content-Encoding: gzip`. Alle anderen Nicht-API-Pfade → 404 (keine Umleitung, kein Captive Portal, D-21).

## 3. `GET /api/status`

Zentrale Statusabfrage, Polling standardmäßig alle 250 ms (nur solange die Seite sichtbar ist). Größenziel ≤ 1 KB.

### Response 200

```json
{
  "ok": true,
  "uptimeMs": 734512,
  "mode": "smart",
  "phase": "pull",
  "pull": {
    "active": true,
    "elapsedMs": 2380,
    "player": "Alex",
    "seq": 17
  },
  "led": {
    "r": 202, "g": 255, "b": 0,
    "brightness": 64,
    "effectiveBrightness": 64,
    "limited": false,
    "on": true
  },
  "config": {
    "mode": "smart",
    "yellowMs": 3000,
    "redMs": 6000,
    "tailMs": 1000,
    "brightness": 64,
    "steadyColor": { "r": 255, "g": 160, "b": 60 },
    "diagnostic": false
  },
  "configVersion": 4,
  "sensor": {
    "levelActive": false,
    "hasPulse": true,
    "lastPulseAgoMs": 140,
    "pulsesInWindow": 23,
    "windowMs": 10000,
    "fault": "none",
    "sampleIntervalMs": 10
  },
  "player": { "current": "Alex" },
  "session": {
    "pullCount": 12,
    "lbVersion": 9,
    "lastPull": {
      "player": "Sam",
      "durationMs": 4710,
      "endedAtMs": 702001,
      "truncated": false,
      "diagnostic": false
    }
  },
  "system": {
    "deviceName": "Smart Shisha LED-Untersetzer",
    "fwVersion": "1.0.0",
    "build": "2026-11-02",
    "ssid": "Shisha-LED-Control",
    "ip": "192.168.4.1",
    "ledCount": 24,
    "fadeMs": 1000,
    "clients": 1
  }
}
```

Feldregeln:

| Feld | Regel |
|---|---|
| `phase` | `idle` \| `pull` \| `fade` (04 §6.1) |
| `pull` | bei `active = false`: `elapsedMs = 0`, `player = null`, `seq` = letzte Zugnummer |
| `led.r/g/b` | logische Farbe vor Helligkeit; bei Modus `off` oder Phase `idle` im Smart-Modus `0,0,0` und `on = false` |
| `sensor.lastPulseAgoMs` | `null`, wenn `hasPulse = false` |
| `sensor.fault` | `none` \| `stuck_active` |
| `session.lastPull` | `null`, wenn noch kein Zug abgeschlossen |
| `system.clients` | Anzahl verbundener WLAN-Stationen laut WLAN-Treiber; nur aufnehmen, wenn in B3 zuverlässig verfügbar, sonst Feld weglassen (WEB-08) |

Freier Heap und Signalstärke werden **nicht** ausgeliefert (keine sinnvolle Interpretation für Nutzer, Lastenheft Screen 7).

## 4. `GET /api/leaderboard`

Wird vom Client nur abgerufen, wenn sich `session.lbVersion` im Status ändert, beim Öffnen des Leaderboard-Screens und nach einem Reset.

### Response 200

```json
{
  "ok": true,
  "lbVersion": 9,
  "uptimeMs": 734512,
  "pullCount": 12,
  "entries": [
    { "rank": 1, "player": "Sam",  "durationMs": 7340, "endedAtMs": 512300, "truncated": false },
    { "rank": 2, "player": "Alex", "durationMs": 4710, "endedAtMs": 702001, "truncated": false }
  ]
}
```

- `entries`: 0–5 Einträge, serverseitig sortiert (Dauer absteigend, bei Gleichstand früher beendet zuerst).
- `uptimeMs` erlaubt dem Client, den Sitzungszeitpunkt relativ darzustellen („vor 3 min“) und optional eine ungefähre Uhrzeit aus der Uhr des Smartphones zu berechnen (D-22).

## 5. `POST /api/config`

Teil-Update der Konfiguration **oder** Reset eines Bereichs.

### 5.1 Request: Teil-Update

Beliebige Teilmenge (mindestens ein Feld):

```json
{
  "mode": "steady",
  "brightness": 80,
  "steadyColor": { "r": 0, "g": 120, "b": 255 },
  "yellowMs": 2500,
  "redMs": 5000,
  "tailMs": 1200,
  "diagnostic": false
}
```

| Feld | Typ | Bereich | Zusatzregel |
|---|---|---|---|
| `mode` | String | `smart`, `steady`, `off` | – |
| `brightness` | Integer | 0–255 | – |
| `steadyColor` | Objekt `{r,g,b}` | je 0–255, alle drei Pflicht | – |
| `yellowMs` | Integer | 500–30.000, Vielfaches von 100 | – |
| `redMs` | Integer | 600–60.000, Vielfaches von 100 | `redMs ≥ yellowMs + 100`, geprüft mit den **resultierenden** Werten (Patch + aktueller Zustand) |
| `tailMs` | Integer | 200–5.000, Vielfaches von 50 | – |
| `diagnostic` | Boolean | – | – |

Bereiche siehe D-09.

### 5.2 Request: Reset

```json
{ "reset": "lighting" }
```

| `reset` | setzt zurück |
|---|---|
| `lighting` | `mode`, `brightness`, `steadyColor` |
| `gradient` | `yellowMs`, `redMs` |
| `sensor` | `tailMs`, `diagnostic` |
| `all` | gesamte Konfiguration (nicht Spielername, nicht Rangliste) |

`reset` darf nicht mit anderen Feldern kombiniert werden (400 `validation_failed`).

### 5.3 Response 200

```json
{ "ok": true, "configVersion": 5, "config": { "...": "vollständige Konfiguration wie in /api/status" } }
```

Wirkung: sofort im RAM; LED-Ausgabe spätestens im nächsten Schleifendurchlauf; ein laufender Zug wird nicht beeinflusst (außer `tailMs`, das sofort für die Endeerkennung gilt).

### 5.4 Fehlerbeispiele

| Anfrage | Antwort |
|---|---|
| `{"redMs": 2000}` bei `yellowMs = 3000` | 400 `validation_failed`, `field: "redMs"` |
| `{"brightness": 300}` | 400 `out_of_range`, `field: "brightness"` |
| `{"brightness": "hoch"}` | 400 `invalid_type` |
| `{"colour": {...}}` | 400 `unknown_field`, `field: "colour"` |
| `{}` | 400 `empty_patch` |
| `{"reset": "all", "mode": "off"}` | 400 `validation_failed` |

## 6. `POST /api/name`

### Request

```json
{ "name": "Alex" }
```

Regeln (04 `utf8_util`):
1. `name` muss ein String sein (`null` wird wie `""` behandelt).
2. Gültiges UTF-8 (keine Überlangkodierung, keine Surrogate, ≤ U+10FFFF), sonst 400 `invalid_name`.
3. Führende/abschließende Leerzeichen werden entfernt.
4. Steuerzeichen U+0000–U+001F und U+007F → 400 `invalid_name`.
5. Mehr als 24 Codepoints → 400 `invalid_name` (kein stilles Abschneiden).
6. Leer nach Trim → gespeichert wird „Unbekannt“, Antwort mit `"normalized": true`.

Der Name gilt für den **nächsten** Zugbeginn; ein laufender Zug behält seinen Namen.

### Response 200

```json
{ "ok": true, "player": "Alex", "normalized": false, "appliesFromNextPull": true }
```

`appliesFromNextPull` ist `true`, wenn gerade ein Zug läuft (UI-Hinweis), sonst `false`.

## 7. `POST /api/leaderboard/reset`

### Request

```json
{ "confirm": true }
```

Fehlt `confirm` oder ist es nicht `true` → 400 `confirmation_required`.

### Wirkung

- Leert alle Einträge, setzt `pullCount` auf 0 (D-11), erhöht `lbVersion`.
- `lastPull` bleibt erhalten (Dashboard-Anzeige), ein laufender Zug läuft weiter und wird nach Abschluss normal eingetragen.
- Konfiguration und Spielername bleiben unverändert.

### Response 200

```json
{ "ok": true, "lbVersion": 10, "pullCount": 0 }
```

## 8. Polling-Strategie des Clients

| Daten | Quelle | Intervall |
|---|---|---|
| Live-Status | `GET /api/status` | 250 ms (Client-Konstante, im System-Screen auf 250/500/1000 ms umschaltbar, gilt nur bis zum Neuladen der Seite) |
| Rangliste | `GET /api/leaderboard` | bei Änderung von `lbVersion` |
| Hintergrund | – | Polling pausiert, wenn `document.visibilityState = "hidden"` |
| Timeout | – | Anfrage abbrechen nach 1.500 ms; 2 Fehlschläge in Folge → Zustand „Verbindung verloren“, Wiederholung mit Backoff 1 s → 2 s → 4 s (max. 5 s) |

Lastabschätzung: 4 Clients × 4 Anfragen/s × ≈ 1 KB ≈ 16 KB/s – für den ESP32-C3 unkritisch, wird in T-LONG-01 bestätigt. Erweist sich die Last als zu hoch, wird das Standardintervall auf 500 ms erhöht.
