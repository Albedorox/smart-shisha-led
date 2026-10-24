# 10 – Offene Entscheidungen, Änderungsanträge, Annahmen, Messaufgaben und Risiken

Stand: 2026-10-24 (Rev. 2, eingearbeitet: Antworten und Messwerte des Nutzers vom 2026-10-24) · Phase A · Solange keine Antwort vorliegt, gilt ein Vorschlag nur als Planungsannahme, nicht als Freigabe.

## 1. Offene Fragen an den Nutzer (Priorität)

| Nr. | Frage | Vorschlag | Warum wichtig |
|---|---|---|---|
| 1 | ✓ entschieden 2026-10-24: ja, alle 24. **Sollen alle 24 LEDs des Rings angesteuert werden?** (CR-01) | ja, alle 24 gleich | Lastenheft nennt 12; betrifft Firmware, Strom, UI |
| 2 | ✓ 2026-10-24: Stufe 1 (keine Zusatzmaßnahme, ESP-Grundlast), keine Messung; bei Bedarf später per Firmware nachrüsten. **Keep-Alive für die Powerbank: zuerst messen, ob die Grundlast reicht, und nur bei Bedarf einen Lichtpuls oder eine unsichtbare Pulslast einbauen?** (CR-02, D-29) | ja, gestuft | neue Anforderung, ggf. Zusatzteile |
| 3 | ✓ 2026-10-24: ja, abschaltbar. **Bereitschaftsglimmen im Smart-Ruhezustand** (z. B. eine LED ganz schwach) gewünscht? (D-30) | ja, abschaltbar | Lastenheft: Smart-Ruhe = aus |
| 4 | ✓ 2026-10-24: Auflage außen max. ca. 113 mm; max. Glas-Ø 150 mm in ca. 30 mm Höhe; 36° zur Glasachse (= 54° zum Tisch); Glas kippelt nicht und lässt sich über dem Ring ca. 13 mm in alle Richtungen verschieben, ohne ihn zu berühren. Glas: maximaler Außendurchmesser? Ist der 36°-Winkel zur Tischfläche oder zur Senkrechten gemessen? Kippelt das Glas, wenn der Ring flach darunter liegt? (M-09) | – | Zentrierrand, Optik, Außendurchmesser |
| 5 | ✓ 2026-10-24: 125 mm. Außendurchmesser des Untersetzers: kompakt (125 mm) oder so groß wie das Glas? (D-31) | 125 mm | Optik, Material, Druckzeit |
| 6 | ✓ 2026-10-24: PrusaSlicer mit eigenen Profilen; Übergabe als STL; Druckempfehlung in 03 §13. Welcher Slicer: Cura (auf deinem Rechner gefunden) oder Anycubic Slicer? | Cura, falls Kobra-Profil vorhanden | Slicer-Prüfung V-10 |
| 7 | ✓ 2026-10-24: GitHub des Nutzers (Zugang wird eingerichtet). Wohin sollen die Dokumente und später Firmware und CAD-Dateien (Ordner auf deinem Rechner oder GitHub-Repository)? (D-26) | Projektordner auf deinem Rechner | dauerhafte Ablage |
| 8 | ✓ 2026-10-24: **Phase B freigegeben** („die nächsten Stufen“). **Freigabe Phase B** (G0) | B0 + B1 zuerst | – |

Erledigt am 2026-10-24: Lichtführung (D-01), SolidCraft-Verfügbarkeit (D-16), Teilemaße (M-01…M-04, M-06), Drucker (M-08), Versorgungskonzept Powerbank (D-04).

## 2. Änderungsanträge gegenüber Lastenheft v1.0

| ID | Änderung | Grund | Auswirkung | Status |
|---|---|---|---|---|
| CR-01 | 24 statt 12 LEDs (WS2812 5050) | vorhandener Ring | `LED_COUNT = 24`, Strombudget (02 §5), Stromgrenze 500 mA, UI-Vorschau 24 Punkte, Tests | **bestätigt 2026-10-24** (Nutzer) |
| CR-02 | Keep-Alive für Powerbanks | Nutzerwunsch Powerbank-Betrieb | Firmware-Konstanten, ggf. MOSFET + Widerstand (02 §6) | **bestätigt 2026-10-24: Stufe 1** (nur Grundlast); Stufe 2/3 nur bei Bedarf |
| CR-03 | Glasgeometrie: Auflagering Ø 109,1 mm statt Bodendurchmesser 150 mm; Ring oben aufgesetzt statt Fenster in Glastasche | Messung | 03 Rev. 2, MEC-01, Tests T-CAD-02/07/12 | übernommen (Messwert) |

## 3. Entscheidungsliste

| ID | Thema | Optionen | Vorschlag / Entscheidung | Blockiert |
|---|---|---|---|---|
| D-01 | Lichtführung | V-A von unten durchs Glas · V-B Ring außen | **ENTSCHIEDEN 2026-10-24: V-A** (Nutzer). Umsetzung nach Messung: 24er-Ring oben in Einfassung, liegt in der Wölbung des Glasbodens | – |
| D-02 | Abdeckung des Rings | offen · dünne klare Abdeckscheibe | **offen (ohne Abdeckung)**: maximale Helligkeit, laut Nutzer kein Wasser auf dem Untersetzer; Abdeckung als optionales Teil T5 | CAD |
| D-03 | Pegelanpassung Datenleitung | 74AHCT125 · Diodentrick · direkt | **74AHCT125-Breakout** | Hardware |
| D-04 | Stromquelle | Netzteil · Powerbank | **Powerbank ≥ 1 A, besser 2 A** (Nutzer: Powerbank) | – |
| D-05 | LED-Stromgrenze | 400 · 500 · 800 mA | **500 mA**: mit ESP-Spitzen ca. 0,85 A, Wärme unkritisch; mit 24 LEDs trotzdem heller als ursprünglich geplant | Firmware |
| D-06 | Versorgungspfad | USB direkt am SuperMini · separate USB-C-Einbaubuchse (mit CC-Widerständen) speist SuperMini (5V-Pin) und Ring | **Einbaubuchse** (entspricht dem Wunsch „5-V-Anschluss in die Basis, dort aufteilen“); Programmieren bei geöffnetem Gehäuse, Powerbank dabei abziehen (M-01b) | Hardware, CAD |
| D-07 | Standardfarbe Dauerlicht | Warmweiß (255,160,60) · andere | **Warmweiß** | Firmware |
| D-08 | Maximale Zugdauer | keine · 60 s | **60 s**, Eintrag mit „max.“ markiert | Firmware |
| D-09 | Wertebereiche | siehe 01 §9 | Gelb 0,5–30 s, Rot bis 60 s und ≥ Gelb + 0,1 s, Nachlauf 0,2–5 s | API, UI |
| D-10 | Fade-out-Dauer einstellbar? | fest · konfigurierbar | **fest 1 s** | – |
| D-11 | Leaderboard-Reset setzt Zugzähler zurück? | ja · nein | **ja**; letzter Zug bleibt auf dem Dashboard | API |
| D-12 | Impulserkennung | Abtasten · Abtasten + Interrupt-Latch | **mit Latch** | Firmware |
| D-13 | Verbindungstechnik | Löten · Stecker; Einschmelzmuttern | **Kabel direkt angelötet** (Nutzer einverstanden), **M2-Einschmelzmuttern für den Ring, M3 für Gehäuse und Sensor** | CAD |
| D-14 | Neustart-Funktion in der UI | nein · ja | **nein in V1** | – |
| D-15 | Standfläche | Silikonpuffer · Moosgummiring | **Puffer**, Ring bei unebenen Tischen | CAD |
| D-16 | CAD-Werkzeug | SolidCraft · CadQuery | **ENTSCHIEDEN: SolidCraft** über den Rechner des Nutzers (geprüft 2026-10-24, 08) | – |
| D-17 | Bibliotheken | eingebaut `WebServer` + Adafruit NeoPixel · Async + FastLED | **eingebaut + NeoPixel** | Firmware |
| D-18 | JSON | ArduinoJson für POST · eigener Parser | **ArduinoJson nur für Eingaben** | Firmware |
| D-19 | Dateinamen | `main.cpp`/`WebServer` · `.ino`/`WebInterface` | **`.ino` + `WebInterface`** | Firmware |
| D-20 | Gammakorrektur | nein · ja | **nein in V1** | – |
| D-21 | Captive Portal | nein · ja | **nein in V1** | – |
| D-22 | Anzeige Sitzungszeitpunkt | „+mm:ss seit Start“ · zusätzlich „vor X min“ | **beides**, keine Uhrzeit | UI |
| D-23 | Übernahme Farbverlauf | Button · automatisch | **Button** | UI |
| D-24 | Klickbarer Mock-up | ja · nein | **optional** | – |
| D-25 | Material | PETG · PLA | **PETG** (PLA möglich, erweicht aber früher) | CAD |
| D-26 | Ablageort | Rechner des Nutzers · Repository · nur Projekt | **ENTSCHIEDEN: GitHub des Nutzers** | – |
| D-27 | Mindestdauer für Ranglisteneinträge | keine · 1,5 s | **keine** | Firmware |
| D-28 | Phase B | ganz · in Stufen | **in Stufen** (11) | alles |
| D-29 | Keep-Alive Powerbank | keiner · Lichtpuls · Pulslast | **ENTSCHIEDEN: Stufe 1**, keine Zusatzteile, keine Messung; Firmware hält eine Erweiterung vor | Hardware, Firmware |
| D-30 | Bereitschaftsglimmen | aus · eine LED sehr schwach | **ENTSCHIEDEN: ja, abschaltbar** (Config `idleGlow`, Standard an) | Firmware, UI |
| D-31 | Außendurchmesser | 125 mm · ≈ Glas-Ø | **ENTSCHIEDEN: 125 mm** | CAD |

## 4. Annahmen

| ID | Annahme | Prüfung |
|---|---|---|
| A-01 | Glas steht stabil auf dem Auflagering; Wölbung lässt über Ring und Zentrierrand genug Luft | M-09, T-CAD-07 |
| A-02 | Ring-Platine 1,6 mm, LED 1,6 mm hoch | M-03b |
| A-03 | GPIO 0/4 elektrisch frei nutzbar (Händler-Pinout belegt Herausführung) | T-HW-01 |
| A-04 | SW-420-Modul an 3,3 V betreibbar, Open-Collector-Ausgang | T-HW-04 |
| A-05 | 20 mA je LED-Kanal bei 255 | M-05 |
| A-06 | Shisha befüllt 2–3 kg | – |
| A-07 | Kobra 2 Neo Bauraum 220 × 220 mm | Herstellerangabe |
| A-08 | Smartphones bleiben mit AP ohne Internet verbunden | T-INT-01 |
| A-09 | ESP-Grundlast (80–120 mA) hält die Powerbank wach | M-10 |

## 5. Messaufgaben

| ID | Aufgabe | Status |
|---|---|---|
| M-01 | Pinout | **erledigt** 2026-10-24 |
| M-01b | Diode zwischen USB-VBUS und 5V-Pin? | offen |
| M-02 | SuperMini-Maße | **erledigt** |
| M-03 / M-03b | Ring-Maße / Höhen | **erledigt** / offen |
| M-04 / M-04b | SW-420-Maße / Bauhöhe | **erledigt** / offen |
| M-05 | Stromaufnahme realer Aufbau | B0/B2 |
| M-06 | Glas: Auflage, Wölbung, Gewicht | **erledigt** |
| M-07b | USB-C-Einbaubuchse | nach Kauf |
| M-08 | Drucker | **erledigt** |
| M-09 | Glas: max. Ø, Bezug 36°, Freiraum der Wölbung / Kippel-Test | **erledigt** 2026-10-24 |
| M-10 | Powerbank: Abschaltverhalten | entfällt (Nutzer, 2026-10-24) |

## 6. Risikobewertung

W = Wahrscheinlichkeit, A = Auswirkung (1 gering … 3 hoch).

| ID | Risiko | W | A | Maßnahme |
|---|---|---|---|---|
| R-01 | Fehltrigger (Abstellen, Stöße, Schlauch); Sensor misst keinen Luftstrom | 3 | 2 | dokumentieren, Diagnosescreen, Poti-Kalibrierung, Sensor starr an der Deckplatte |
| R-02 | **SW-420 erkennt das Blubbern durch Glas und Untersetzer nicht zuverlässig** | 2 | 3 | **Machbarkeitstest B0 zuerst**; Alternativen als Änderungsantrag (andere Position, Piezo-Scheibe) |
| R-03 | Glas kippelt auf dem Ring oder berührt die LEDs | 1 | 2 | Ring trägt nicht (Auflage außen), Zentrierrand, Kippel-Test M-09, Testring T-CAD-07 |
| R-04 | Überstrom / Brownout an der Powerbank | 1 | 2 | Stromgrenze 500 mA, Elko, Polyfuse |
| R-05 | 3,3-V-Datensignal unzuverlässig | 2 | 2 | Pegelwandler |
| R-06 | Flüssigkeit an der Elektronik | 1 | 2 | Kabelloch abdichten, Elektronik im unteren Teil, Ablauföffnungen |
| R-07 | WLAN-Probleme der SuperMini-Klone | 2 | 2 | Sendeleistung als Konstante, Antenne zur Außenwand, T-HW-09 |
| R-08 | Erweichen des Kunststoffs (PLA im Sommer/Auto) | 1 | 2 | PETG |
| R-09 | Kippen der Shisha | 1 | 3 | niedrige Bauhöhe ca. 19 mm, Ø 125 > Auflage 109,1 |
| R-10 | Fremde ändern Einstellungen (offenes WLAN) | 2 | 1 | dokumentiert |
| R-11 | SolidCraft-Funktionen weichen von der Katalogbeschreibung ab | 1 | 2 | Funktionstests V-02…V-10 vor dem Produktmodell |
| R-12 | Smartphone trennt den AP („kein Internet“) | 2 | 2 | Hinweis in Hilfe, Tests iOS/Android |
| R-13 | `WebServer`-Bibliothek nutzt dynamische Strings | 2 | 2 | Größenlimit, Langzeittest |
| R-14 | Licht dringt nicht durch den Glasboden | 1 | 2 | **weitgehend ausgeräumt**: Fotos zeigen klaren Boden mit eingelegtem Ring |
| R-15 | Rückspeisung USB ↔ Powerbank beim Programmieren | 2 | 2 | M-01b; bis dahin Powerbank beim Flashen abziehen |
| R-16 | Wärme im Dauerlicht | 1 | 2 | Stromgrenze, T-HW-07 |
| R-17 | Powerbank schaltet trotz Keep-Alive ab | 2 | 2 | M-10, Stufe 3 (Pulslast), Hinweis auf geeignete Powerbanks |
| R-18 | USB-C-zu-USB-C ohne CC-Widerstände liefert keine Spannung | 2 | 2 | Breakout mit 5,1-kΩ-CC-Widerständen (E8) |

## 7. Spannungsfelder im Lastenheft (Vorschlag zur Auflösung)

| Stelle | Spannungsfeld | Auflösung |
|---|---|---|
| §5.1 `main.cpp`, Modul `WebServer` | Arduino IDE verlangt `.ino`; Klassenname kollidiert | D-19 |
| §6.2 Screen 4 „unmittelbare Anwendung“ vs. Reihenfolgeprüfung | ungültige Zwischenzustände | D-23 |
| §3.3 / §4.1 12 LEDs, Glas Ø 150 | reale Teile weichen ab | CR-01, CR-03 |
| §2.2 kein Akkumanagement vs. Powerbank-Betrieb | Powerbank bringt BMS mit; nur Keep-Alive ist neu | CR-02 |
| §5.4 Smart-Ruhe „LEDs aus“ vs. Bereitschaftsglimmen / Lichtpuls | – | D-30, D-29 (nur mit Zustimmung) |
| §7 sechs Endpunkte vs. Konfigurationsabfrage und Resets | – | Konfiguration in `/api/status`, Resets über `POST /api/config {"reset": …}` |
| §6.2 Screen 3 Impulsanzeige vs. 250-ms-Polling | kurze Pegelwechsel unsichtbar | Flankenzähler im Gerät |
| §8 Abtastintervall als Konfigurationswert | ohne UI-Vorgabe | fest 10 ms, nur angezeigt |
