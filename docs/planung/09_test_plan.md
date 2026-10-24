# 09 – Testplan: Hardware, Firmware, API, UI, CAD

Stand: 2026-10-23 · Phase A · Die Tests werden in Phase B ausgeführt. Jede Zeile nennt die geprüften Anforderungen (01) und ein eindeutiges Bestanden-Kriterium.

## 1. Testebenen und Hilfsmittel

| Ebene | Ort | Hilfsmittel |
|---|---|---|
| Host-Tests (Logik) | PC/Arbeitsumgebung, g++ | reine C++-Module mit simulierter Zeit (04 §2.2) |
| Hardware | Werkbank | Multimeter, USB-Strommessgerät, optional Logikanalysator (≥ 4 MHz), Thermometer/IR |
| Firmware auf Gerät | ESP32-C3 | serielles Log (USB-CDC), Testschalter `LOG_LEVEL=DEBUG` |
| API | Laptop/Smartphone im AP | curl-Skript `tools/api_tests.sh` (Phase B), Ausgabe Soll/Ist |
| UI | Smartphones | iPhone (Safari), Android (Chrome), Browser-Entwicklertools (Netzwerk, Viewport) |
| Langzeit | Gerät + 3 Clients | Polling-Skript + serielles Heap-/Schleifenzeit-Log |
| CAD | Arbeitsumgebung + Drucker | Netzprüfung, Slicer-CLI, Testdrucke |
| Review | Code/Doku | Checklisten |

Testprotokoll: Datum, Firmware-Version/Commit, Hardware-Stand, Ergebnis (bestanden/nicht bestanden/blockiert), Messwerte, Bemerkung.

## 2. Hardwaretests

| ID | Test | Anf. | Durchführung | Bestanden, wenn |
|---|---|---|---|---|
| T-HW-01 | Pinout-Verifikation | HW-01 | Board fotografieren, Aufdruck mit Pinout vergleichen; Testsketch schaltet GPIO 4 im 1-Hz-Takt, Messung am Pin; GPIO 0 als Eingang mit Pull-up liest Taster | Beide Pins funktionieren wie vorgesehen; Bootvorgang mit angeschlossenem Sensor (in beiden Pegeln) normal |
| T-HW-02 | Versorgung und Masse | HW-02 | Durchgang GND zwischen ESP, LED, Sensor, Pegelwandler; LED-5V nicht an GPIO | Durchgang < 1 Ω; Schaltplan entspricht Aufbau |
| T-HW-03 | Datensignal-Pegel | HW-03 | Pegel am LED-DIN mit Logikanalysator/Oszilloskop; 1 h Farbwechseltest | High ≥ 3,5 V; keine Fehlfarben/Flackern in 1 h |
| T-HW-04 | Sensorpolarität und Pegel | HW-04 | Multimeter an DO in Ruhe und beim Klopfen; Impulsbreiten mit Logikanalysator | DO ≤ 3,6 V; Polarität dokumentiert; `SENSOR_ACTIVE_LOW` passend gesetzt; kürzeste Impulsbreite dokumentiert |
| T-HW-05 | Stromaufnahme | SYS-02, HW-05, LED-10 | USB-Messgerät: Leerlauf, Smart bei Zug (64), Dauerlicht Weiß 64/128/255 | Messwerte dokumentiert; Maximalstrom ≤ Budget D-04; Stromgrenze greift (≤ Limit + 10 %) |
| T-HW-06 | LED-Einzeladressierung | LED-01 | Testmuster: jede LED einzeln R, G, B | alle 24 LEDs in allen Farben korrekt, Reihenfolge dokumentiert (Index 0 = DIN-nächste LED) |
| T-HW-07 | Thermik | 02 §7 | 60 min Dauerlicht mit Stromgrenze, im geschlossenen Gehäuse | Gehäuseoberfläche ≤ 40 °C, LED-Ring ≤ 60 °C, ESP ≤ 70 °C |
| T-HW-08 | Zugentlastung, Isolierung | SAF-03 | 20 N Zug am Versorgungskabel; Sichtprüfung | keine Bewegung an Lötstellen; keine blanken Leiter |
| T-HW-10 | Powerbank-Dauerbetrieb | SYS-02, D-29 | Powerbank des Nutzers, Smart-Modus ohne Züge 60 min, danach 4 h gemischt | keine Abschaltung; gewählte Keep-Alive-Stufe und Restkapazität dokumentiert |
| T-HW-09 | WLAN-Reichweite / Sendeleistung | R-07 | Verbindung bei 1 m, 3 m, 5 m im eingebauten Zustand, Sendeleistung 8,5 dBm und höher | stabile Verbindung (keine Abbrüche in 5 min) bei 3 m; gewählte Sendeleistung dokumentiert |

## 3. Firmware-Tests (Host und Gerät)

| ID | Test | Anf. | Durchführung | Bestanden, wenn |
|---|---|---|---|---|
| T-FW-01 | Abtastraster | SEN-01 | Gerät: Zeitstempel der Auswertungen 60 s loggen | Mittel 10 ms ± 2 ms; kein Mehrfachnachholen nach Verzögerung |
| T-FW-02 | Zugstart | SEN-02 | Host: Impuls bei t = 100 | `active` ab Auswertung ≤ 110 |
| T-FW-03 | Zug verlängern | SEN-03 | Host: Impulse alle 900 ms über 10 s | ein Zug, aktiv bis letzter Impuls + 1.000 |
| T-FW-04 | Zugende | SEN-04 | Host: letzter Impuls bei t = L | Ende bei L + `tailMs` (+0…+10 ms) |
| T-FW-05 | Zugdauer Einzelimpuls | SEN-05 | Host: ein Impuls | `durationMs` = 1.000 ± 10 (Host) bzw. ± 20 (Gerät) |
| T-FW-06 | Genau-einmal | SEN-06 | Host: 100 Züge, dabei abwechselnd Status-/Leaderboard-Abfragen simulieren | `pullCount` = 100, keine Doppel- oder Fehleinträge |
| T-FW-07 | Dauersignal | SEN-09 | Host: Pegel dauerhaft aktiv 70 s | `fault = stuck_active` nach 5 s; Zug endet bei 60 s mit `truncated`; kein neuer Zug bis Pegel ≥ `tailMs` inaktiv |
| T-FW-08 | Diagnosemodus | SEN-10 | Host: `diagnostic = true`, 3 Züge | Rangliste und `pullCount` unverändert; `lastPull.diagnostic = true` |
| T-FW-10 | Farbverlauf | LED-02, LED-06 | Host: `gradient(t)` für Referenzwerte 04 §6.2 sowie für Y=500/R=600 und Y=30.000/R=60.000 | Abweichung ≤ 1 je Kanal; monoton (R steigend, dann G fallend) |
| T-FW-11 | Moduswechsel im Zug | LED-05 | Host: Zug, nach 2 s Wechsel Smart→Steady→Off→Smart | `durationMs` identisch zum Lauf ohne Wechsel; Farbe nach Rückwechsel = `gradient(elapsed)` |
| T-FW-12 | Fade-out | LED-08 | Host: Faktor bei 0/250/500/750/1.000 ms | 255/191/127/64/0 (± 3) |
| T-FW-13 | Neuer Zug im Fade | LED-09 | Host: Impuls 400 ms nach Zugende | Phase `pull`, Farbe (0,255,0) |
| T-FW-14 | Stromgrenze | LED-10 | Host: Weiß, B = 255, Limit 500, 24 LEDs | `B_eff` = 255·500/1.440 = 88 (± 1); `limited = true` |
| T-FW-15 | Rangliste Kapazität | SES-01, SES-06 | Host: 8 Züge mit Dauern 1…8 s | 5 Einträge (8,7,6,5,4 s); `pullCount` = 8 |
| T-FW-16 | Gleichstand | SES-02 | Host: Züge A (3.000), B (3.000), C (3.000) nacheinander | Reihenfolge A, B, C |
| T-FW-17 | Reset im Zug | SES-03 | Host: Zug starten, Reset, Zug beenden | Rangliste enthält genau diesen Zug, `pullCount` = 1 |
| T-FW-18 | Name bei Zugbeginn | SES-05 | Host: Name „A“, Zugstart, Name „B“, Zugende | Eintrag mit „A“; nächster Zug „B“ |
| T-FW-19 | UTF-8-Validierung | SES-04 | Host: Tabelle mit ASCII, Umlauten, 4-Byte-Emoji, 24/25 Codepoints, ungültigen Folgen (C0 80, ED A0 80, F5…), Steuerzeichen, nur Leerzeichen | erwartete Annahme/Ablehnung/Normalisierung je Fall |
| T-FW-20 | Keine Persistenz | SYS-03 | Gerät: Werte ändern, Strom trennen, neu starten | alle Werte = Standard |
| T-FW-21 | Nicht blockierend | SYS-04 | Gerät: maximale Schleifendauer loggen, während `/` geladen und gepollt wird | Max < 50 ms, Ziel < 20 ms; keine verpassten Züge (Latch) |
| T-FW-22 | `millis()`-Überlauf | NF | Host: Simulation mit Startzeit 2³² − 3.000 ms über den Überlauf | Zugdauer, Fade und Rangliste korrekt |

## 4. API-Tests

| ID | Test | Anf. | Bestanden, wenn |
|---|---|---|---|
| T-API-01 | `GET /` | API-01 | 200, `Content-Encoding: gzip`, Seite rendert |
| T-API-02 | `GET /api/status` Schema | API-01, SES-07 | alle Felder gemäß 06 §3, Typen korrekt, Größe ≤ 1 KB |
| T-API-03 | `GET /api/leaderboard` | API-01 | Schema und Sortierung gemäß 06 §4 |
| T-API-04 | Moduswechsel | LED-03/04 | 200; `configVersion` +1; Status spiegelt neuen Modus |
| T-API-05 | `tailMs` | SEN-08 | 200 bei 200/5.000; 400 bei 150/5.050/1.025 |
| T-API-06 | Reihenfolge Gelb/Rot | LED-07 | 400 `validation_failed` bei `redMs ≤ yellowMs + 99`; 200 bei gemeinsamem gültigem Patch |
| T-API-07 | Alles-oder-nichts | API-02 | Patch mit einem gültigen und einem ungültigen Feld → 400 und keine Änderung |
| T-API-08 | Name | SES-04 | Fälle aus T-FW-19 über HTTP; `appliesFromNextPull` korrekt |
| T-API-09 | Grenzfälle Protokoll | API-03 | 413 bei 513 Byte; 415 ohne JSON-Content-Type; 400 bei kaputtem JSON; 405 bei GET auf POST-Route; 404 unbekannt; Gerät bleibt stabil (100 Wiederholungen) |
| T-API-10 | Leaderboard-Reset | SES-03 | 400 ohne `confirm`; 200 mit; Einträge leer, `pullCount` 0, `lastPull` erhalten |

## 5. Integrationstests

| ID | Test | Anf. | Bestanden, wenn |
|---|---|---|---|
| T-INT-01 | Offline-Betrieb | SYS-01, WEB-01 | Smartphone im Flugmodus + WLAN: Verbindung, Seite, alle Funktionen |
| T-INT-02 | Realer Zug | SEN-02…05 | 10 echte Züge an der Shisha; Dauer plausibel (Stoppuhr ± 0,5 s zuzüglich Nachlauf) |
| T-INT-03 | Bedienbarkeit im Zug | SEN-07 | während Zügen: Status p95 < 150 ms, POSTs < 300 ms |
| T-INT-04 | Dauerlicht | LED-03 | Farbe ≤ 100 ms nach 200-Antwort sichtbar; Züge werden erfasst |
| T-INT-05 | Modus Aus | LED-04 | LEDs dunkel; Züge in Rangliste |
| T-INT-06 | Fehltrigger-Bewertung | R-01 | Glas abstellen, Tisch anstoßen, Schlauch bewegen: Auslösungen dokumentieren | Ergebnis dokumentiert (kein hartes Kriterium; Grundlage für Kalibrierungsanleitung) |

## 6. UI-Tests

| ID | Test | Anf. | Bestanden, wenn |
|---|---|---|---|
| T-UI-01 | Keine externen Ressourcen | WEB-02 | Netzwerk-Log nur 192.168.4.1; Quelltextsuche nach `http` ohne Treffer außer Hilfetext-IP |
| T-UI-02 | Navigation | WEB-03 | alle 8 Screens in ≤ 2 Taps; Zurück-Taste funktioniert |
| T-UI-03 | Responsiv | WEB-04 | 320, 360, 390, 430 px hoch und quer, 1024 px: kein horizontales Scrollen, Touch-Ziele ≥ 44 px |
| T-UI-04 | Validierung Farbverlauf | LED-07 | Fehlermeldung bei Rot ≤ Gelb, „Übernehmen“ deaktiviert |
| T-UI-05 | Live-Dauer | WEB-05 | sichtbare Aktualisierung ≥ 3×/s |
| T-UI-06 | Verbindungsverlust | WEB-06 | AP abschalten: Banner ≤ 2 s, Werte ausgegraut, Zähler stoppt; Wiederverbindung automatisch |
| T-UI-07 | Kontrast | WEB-07 | alle Texte ≥ 4,5 : 1 (Messung mit Kontrastprüfer) |
| T-UI-08 | Keine vorgetäuschten Werte | WEB-08 | Feld aus API entfernen (Testbuild) → „nicht verfügbar“ |
| T-UI-09 | Bestätigungen | WEB-09 | Reset Rangliste/Konfiguration nur über Dialog |
| T-UI-10 | Hinweise | SAF-01 | Hinweis offenes WLAN und Datenverlust in Hilfe und System vorhanden |
| T-UI-11 | Namenseingabe | SES-04 | Zähler in Codepoints; Emoji-Fälle; Name wird per `textContent` dargestellt (`<script>` als Name erscheint als Text) |
| T-UI-12 | Mehrere Clients | 05 §3 | Änderung auf Gerät A erscheint auf Gerät B ≤ 1 s mit Hinweis |

## 7. Langzeit- und Lasttest

| ID | Test | Bestanden, wenn |
|---|---|---|
| T-LONG-01 | 8 h Betrieb: 3 Clients pollen mit 250 ms, alle 10 s ein POST, simulierte Züge alle 20 s (Testsignal an GPIO 0 oder Klopfgerät) | kein Neustart/Absturz; minimaler freier Heap stabil (Abnahme < 2 KB zwischen Stunde 1 und 8); max. Schleifendauer < 50 ms; alle Züge erfasst |

## 8. Reviews

| ID | Prüfung | Bestanden, wenn |
|---|---|---|
| T-REV-01 | Persistenz | keine Persistenz-APIs im Code (Suche) |
| T-REV-02 | Keine Platzhalter-Messwerte | API-Antworten ausschließlich aus Zustandsvariablen |
| T-REV-03 | Sicherheitshinweise | Anleitung enthält Material-, Hitze-, Wasser- und Stromhinweise |

## 9. CAD- und Fertigungstests

| ID | Test | Anf. | Bestanden, wenn |
|---|---|---|---|
| T-CAD-01 | Werkzeugverifikation | 08 §3 | Protokoll V-01…V-10 ausgefüllt |
| T-CAD-02 | Parametrik | MEC-02 | `glass_contact_d` 100/109,1/120 und `outer_d` 115/125/150 erzeugen gültige Modelle |
| T-CAD-03 | Vollständigkeit | MEC-03 | alle Baugruppen + Stückliste vorhanden |
| T-CAD-04 | Netzgültigkeit | MEC-04 | alle STL/3MF geschlossen, mannigfaltig |
| T-CAD-05 | Wandstärke/Überhänge | MEC-04 | ≥ 1,6 mm (Fenster 1,0); keine Stützen nötig für T1–T4 |
| T-CAD-06 | Slicer | MEC-04 | Slicer-CLI ohne Warnung; Druckzeit/Filament dokumentiert |
| T-CAD-07 | Glas-Testring | MEC-01 | reales Glas passt ohne Klemmen; Spiel ≤ 2 mm je Seite |
| T-CAD-08 | Toleranzcoupon | 03 §7 | Passungswerte bestätigt oder Parameter korrigiert |
| T-CAD-09 | Montage/Demontage | MEC-05 | 3 Zyklen ohne Beschädigung, Einsätze halten |
| T-CAD-10 | Spritzwasser | MEC-06 | 50 ml in die Tasche, 15 min: Elektronikfach trocken |
| T-CAD-11 | Standfestigkeit | MEC-07 | kein Rutschen bei leichtem Stoß auf Glasplatte; kein Kippeln |
| T-CAD-12 | Statische Last | 03 §9 | 5 kg, 24 h: Durchbiegung < 0,5 mm, keine bleibende Verformung |

## 10. Dokumentation

| ID | Test | Bestanden, wenn |
|---|---|---|
| T-DOC-01 | Nachvollziehbarkeit | eine zweite Person baut nach Dokumenten 12–15 auf und nimmt in Betrieb, Rückfragen dokumentiert |

## 11. Rückverfolgbarkeit

Jede Anforderung in 01 verweist auf mindestens einen Test. Zusätzliche Tests ohne direkte Anforderungs-ID (T-FW-19, T-FW-22, T-INT-02/06, T-UI-11/12) prüfen nicht-funktionale Anforderungen aus Lastenheft §9.
