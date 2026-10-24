# 11 – Umsetzungsplan: Reihenfolge, Abhängigkeiten, Freigabepunkte

Stand: 2026-10-24 (Rev. 2) · Phase A · **Keiner der folgenden Schritte beginnt ohne ausdrückliche Freigabe (G0).**

## 1. Freigabepunkte

| ID | Freigabe | Voraussetzung | Wer |
|---|---|---|---|
| **G0** | Phase B beginnt; Planungsdokumente 01–11 inkl. Wireframes und Entscheidungen D-01…D-28 bestätigt | Prüfung durch Nutzer | Nutzer |
| G1 | Hardware bestätigt: Pinout, Sensorpolarität, Machbarkeit Zugerkennung (R-02), Lichtwirkung (R-14) | B0 abgeschlossen, M-01/M-04/M-06 | Nutzer |
| G2 | Firmware-Kern und API stehen; UI-Umsetzung darf beginnen | B1–B3 Tests bestanden | Nutzer (Kurzprüfung) |
| G3 | CAD-Entwurf geprüft; Export und Testdrucke dürfen erfolgen | B5 Prüfungen 03 §12, Ansichten vorgelegt | Nutzer |
| G4 | Testdrucke bestanden; Vollteile drucken | T-CAD-07/08 | Nutzer |
| G5 | Endabnahme V1 | alle Tests aus 09 bestanden oder Abweichung akzeptiert | Nutzer |

## 2. Arbeitspakete

| Paket | Inhalt | Ergebnis | Abhängig von | Tests |
|---|---|---|---|---|
| **B0 Hardware-Verifikation** | Board identifizieren, Pinout prüfen, Sensor an 3,3 V testen (Polarität, Impulsbreite), Machbarkeitstest Zugerkennung am echten Glas, LED-Ring unter das Glas halten (Lichtwirkung), Strommessung mit Testsketch, Abschaltverhalten der Powerbank (M-10), Kippel-Test Glas auf Ring (M-09). Testsketches sind Wegwerfcode, nicht Teil der Firmware | Messprotokoll, aktualisierte 02/03/10 | G0, Hardware beim Nutzer vorhanden | T-HW-01, -04, -06, R-02/R-14-Test |
| B1 Firmware-Kern (logisch) | `app_config.h`, `time_util`, `color_math`, `utf8_util`, Zustandsmaschine, Rangliste, ConfigManager; Host-Tests | Module + grüne Host-Tests | G0 (Logik), G1 (Pins/Polarität für Konstanten) | T-FW-02…19, -22 |
| B2 Firmware-Hardwareanbindung | SensorManager mit ISR-Latch, LedController mit NeoPixel und Stromgrenze, Hauptschleife, serielles Log | Gerät erkennt Züge, LEDs folgen der Logik | B1, G1 | T-FW-01, -20, -21, T-HW-03, -05, -07 |
| B3 WLAN und API | AP, Routen, Validierung, Fehlercodes, JSON | API vollständig gemäß 06 | B2 | T-API-01…10 |
| B4 Weboberfläche | `web/index.html` (8 Screens gemäß 05/07), Build-Skript gzip → `web_assets.h` | lauffähige Oberfläche vom Gerät | G2 (B3), freigegebene Wireframes | T-UI-01…12, T-INT-01…06 |
| B5 CAD (SolidCraft auf dem Rechner des Nutzers) | Funktionstests (08 §3), Parameterdatei, Teile T1–T6, Hüllkörper, Baugruppe, Ansichten, Stückliste | prüfbares Modell + Ansichten | G0, D-01/D-02/D-16, M-02/03/04/06/07/08 | T-CAD-01…05 |
| B6 Fertigung | Export STL/3MF + Quellformat/STEP, Slicer-Lauf, Toleranzcoupon, Glas-Testring, Vollteile | druckbare Dateien, gedruckte Teile | G3, dann G4 | T-CAD-06…12 |
| B7 Integration und Langzeit | Montage, Gesamttest, 8-h-Test, Fehltrigger-Bewertung | Testprotokoll | B4, B6 | T-LONG-01, T-HW-08/09, T-INT-02/06 |
| B8 Dokumentation | 12_montage.md, 13_verkabelung.md, 14_kalibrierung.md, 15_inbetriebnahme.md, Bedienhinweise | vollständige Anleitung | B7 | T-DOC-01, T-REV-03 |

## 3. Abhängigkeitsgraph

```
G0 ─┬─► B0 ─► G1 ─┬─► B2 ─► B3 ─► G2 ─► B4 ─┐
    │             │                          ├─► B7 ─► B8 ─► G5
    ├─► B1 ───────┘                          │
    │                                        │
    └─► B5 (nach Messwerten M-0x) ─► G3 ─► B6 ─► G4 ┘
```

- B1 kann sofort nach G0 parallel zu B0 beginnen (reine Logik, Host-Tests).
- B5 kann parallel zu B1–B4 laufen, sobald die mechanischen Messwerte vorliegen und D-01/D-02/D-16 entschieden sind.
- Kritischer Pfad: **B0 (Machbarkeit Sensor)** – scheitert die Zugerkennung, ändert sich das Konzept (Änderungsantrag), bevor Aufwand in CAD und UI fließt.

## 4. Lieferumfang Phase B

| Bereich | Dateien |
|---|---|
| Firmware | `ShishaLed/` (Sketch-Ordner gemäß 04 §2.2), `test/host/`, `tools/build_web_assets.py`, `README.md` mit Board-Paket-Version und Bibliotheksversionen |
| Weboberfläche | `web/index.html` (Quelle), generiertes `web_assets.h` |
| CAD | Parameterdatei, Quellmodell (SolidCraft-Format oder CadQuery-Skript), STEP, STL/3MF je Teil, Montageansichten (PNG), Stückliste |
| Dokumentation | 12–15, Testprotokolle, aktualisierte 01–11 |

## 5. Arbeitsregeln für Phase B

- Vor jedem Bearbeiten bestehender Dateien, jedem Push, Merge oder Pull Request wird gefragt, sofern nicht für den jeweiligen Schritt freigegeben.
- Abweichungen von dieser Planung werden als Änderung in 10 dokumentiert und vor Umsetzung bestätigt.
- Messwerte ersetzen Annahmen nur mit Quelle und Datum.
- Kein CAD-Export vor G3; keine Vollteile vor G4.

## 6. Grobe Aufwandsschätzung (Orientierung, keine Zusage)

| Paket | Aufwand |
|---|---|
| B0 | 1–2 Abende beim Nutzer (Messen, Testaufbau) |
| B1–B3 | 1–2 Arbeitssitzungen |
| B4 | 1–2 Arbeitssitzungen |
| B5 | 1–2 Arbeitssitzungen plus Abstimmung |
| B6 | Druckzeit: Coupon/Testring je ca. 1 h, Deckplatte und Unterschale je ca. 4–8 h (abhängig vom Drucker) |
| B7–B8 | 1 Arbeitssitzung plus 8-h-Test |
