# 01 – Konsolidierte Anforderungen und Abnahmekriterien

Projekt: Smart Shisha LED-Untersetzer · Grundlage: Lastenheft v1.0 · Stand: 2026-10-24 (Rev. 2: 24 LEDs, Glasmaße, Powerbank) · Phase A (Planung)

Dieses Dokument fasst das Lastenheft in eindeutig nummerierte, prüfbare Anforderungen zusammen. Jede Anforderung hat ein Abnahmekriterium. Die Test-IDs verweisen auf `09_test_plan.md`, offene Punkte (D-, A-, M-, R-) auf `10_open_questions.md`.

## 1. Begriffe

| Begriff | Definition (verbindlich für alle Dokumente) |
|---|---|
| Impuls | Ein 10-ms-Auswertezyklus, in dem der Sensor als *aktiv* gilt (Pegel aktiv oder seit dem letzten Zyklus per Interrupt-Latch eine aktive Flanke erkannt, siehe 04 §4). |
| Zug | Zeitraum vom ersten Impuls bis zum erkannten Zugende. |
| Nachlaufzeit (`tailMs`) | Zeit ohne Impuls, nach der ein Zug als beendet gilt. Standard 1.000 ms. |
| Zugdauer (`durationMs`) | `Zeitpunkt der Zugende-Erkennung − Zeitpunkt des ersten Impulses`. Enthält die Nachlaufzeit. |
| Sitzung | Zeitraum seit dem letzten Start der Firmware. Alle Daten sind flüchtig. |
| Sitzungszeitpunkt | Millisekunden seit Start (`millis()`-basiert), keine Kalenderzeit. |
| Phase | Anzeigezustand: `idle` (Bereit), `pull` (Zug erkannt), `fade` (Fade-out, nur im Modus Smart). |

## 2. Systemanforderungen

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| SYS-01 | Die Firmware läuft vollständig auf einem ESP32-C3 SuperMini ohne Cloud oder Internet. | Alle Funktionen sind mit einem Smartphone im Flugmodus mit WLAN nutzbar. | T-INT-01 |
| SYS-02 | Versorgung über 5 V USB-C, bevorzugt aus einer Powerbank; 3–4 h je Session. | Gerät läuft an der Powerbank des Nutzers 4 h im Smart-Modus, ohne dass die Powerbank abschaltet (D-04, D-29). | T-HW-05, T-HW-10 |
| SYS-03 | Keine Persistenz von Konfiguration, Name, Modus oder Rangliste. | Nach Stromtrennung gelten wieder alle Standardwerte (01 §9); Code enthält keine Aufrufe von `Preferences`, `EEPROM`, `nvs_*`, `SPIFFS`, `LittleFS`. | T-FW-20, T-REV-01 |
| SYS-04 | Nicht blockierende, kooperative Hauptschleife mit `millis()`. | Kein `delay()` > 1 ms im Laufzeitpfad; maximale Schleifendauer im Langzeittest < 50 ms (Ziel < 20 ms). | T-FW-21, T-LONG-01 |

## 3. Hardware

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| HW-01 | SW-420 DO an GPIO 0 (vorläufig), WS2812B DIN an GPIO 4 (vorläufig). | Pinout am realen Board verifiziert und in 02 als „bestätigt“ markiert, bevor Firmware-Code entsteht (M-01). | T-HW-01 |
| HW-02 | LEDs werden nicht aus einem GPIO gespeist; gemeinsame Masse ESP32/LED. | Schaltplan in 02 zeigt LED-5V direkt von der 5-V-Schiene; Durchgangsprüfung GND. | T-HW-02 |
| HW-03 | Pegelanpassung des Datensignals ist bewertet und umgesetzt. | Entscheidung D-03 getroffen; Datensignal am LED-Eingang erfüllt V_IH der eingesetzten LEDs. | T-HW-03 |
| HW-04 | Sensorpolarität und 3,3-V-Kompatibilität sind am konkreten Modul verifiziert. | Gemessene Pegel Ruhe/Erschütterung dokumentiert; DO ≤ 3,6 V. | T-HW-04 |
| HW-05 | Stromaufnahme bleibt im Budget der Versorgung. | Gemessener Gesamtstrom bei Worst Case (Konfiguration aus 02 §5) ≤ Budget aus D-04. | T-HW-05 |

## 4. Sensor und Zugerkennung

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| SEN-01 | Auswertung des Sensors alle 10 ms. | Zeitstempel-Log zeigt Auswerteabstand 10 ms ± 2 ms im Mittel; kein Zyklus wird doppelt nachgeholt. | T-FW-01 |
| SEN-02 | Erster Impuls startet einen Zug. | Simulierter Impuls bei Phase `idle` → `pull.active = true` innerhalb ≤ 20 ms. | T-FW-02 |
| SEN-03 | Jeder weitere Impuls aktualisiert den Zeitpunkt des letzten Signals. | Impulsfolge im Abstand 900 ms hält einen Zug beliebig lange aktiv (bei `tailMs` = 1.000). | T-FW-03 |
| SEN-04 | Zugende nach `tailMs` ohne Impuls. | Ende wird bei `letzterImpuls + tailMs` (+0/+20 ms) erkannt. | T-FW-04 |
| SEN-05 | Zugdauer = erster Impuls bis Zugende-Erkennung (inkl. Nachlauf). | Einzelimpuls ergibt `durationMs` = `tailMs` ± 20 ms. | T-FW-05 |
| SEN-06 | Ein abgeschlossener Zug wird genau einmal verarbeitet. | Pro Zug genau ein Leaderboard-Kandidat und `pullCount` +1, auch bei gleichzeitigen HTTP-Anfragen. | T-FW-06 |
| SEN-07 | Weboberfläche bleibt während eines Zuges bedienbar. | API-Antwortzeit `/api/status` während eines Zuges p95 < 150 ms. | T-INT-03 |
| SEN-08 | Nachlaufzeit über Weboberfläche konfigurierbar. | `tailMs` im Bereich 200–5.000 ms (D-09) änderbar; wirkt ab der nächsten Auswertung. | T-API-05 |
| SEN-09 | Dauerhaft aktiver Sensor wird erkannt und begrenzt. | Nach 5 s ununterbrochen aktivem Pegel meldet der Status `sensor.fault = "stuck_active"`; Zug wird nach `maxPullMs` (D-08) beendet und markiert. | T-FW-07 |
| SEN-10 | Diagnosemodus ohne Änderung der Rangliste. | Bei `diagnostic = true` werden Züge angezeigt, aber weder in die Rangliste noch in `pullCount` übernommen. | T-FW-08 |

## 5. LED und Beleuchtungsmodi

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| LED-01 | 24 adressierbare WS2812 5050 (CR-01 bestätigt 2026-10-24, Lastenheft: 12); Helligkeit 0–255, Standard 64. | Alle 24 LEDs einzeln ansteuerbar (Testmuster in Phase B); Helligkeit wirkt linear auf PWM-Wert. | T-HW-06 |
| LED-02 | Modus Smart: Ruhe = aus; Zugbeginn startet Verlauf; Farbe abhängig von Zugdauer; danach Fade-out 1.000 ms. | Farbwerte zu Zeitpunkten 0/1,5/3/4,5/6/10 s entsprechen Tabelle 04 §6 (± 1 pro Kanal). | T-FW-10 |
| LED-03 | Modus Dauerlicht: konfigurierbare RGB-Farbe und Helligkeit; Sensor bleibt aktiv. | Farbe wird ≤ 100 ms nach erfolgreichem POST angezeigt; Züge werden weiter erfasst. | T-INT-04 |
| LED-04 | Modus Aus: LEDs aus; Sensor, Messung, Webserver, Rangliste laufen weiter. | LEDs dunkel; Züge erscheinen in der Rangliste. | T-INT-05 |
| LED-05 | Moduswechsel sofort, ohne Sensorlogik zu blockieren. | Wechsel während eines Zuges verändert `durationMs` nicht. | T-FW-11 |
| LED-06 | Farbverlauf Grün (0 s) → Gelb (3 s) → Rot (6 s), linear interpoliert, danach Rot. | Siehe LED-02. | T-FW-10 |
| LED-07 | Schwellen konfigurierbar; Rot stets > Gelb. | API lehnt `redMs ≤ yellowMs` mit 400 ab; UI zeigt Fehlermeldung. | T-API-06, T-UI-04 |
| LED-08 | Fade-out: letzte Zugfarbe über 1.000 ms kontinuierlich auf Schwarz, mit `millis()`. | Helligkeitsfaktor bei 0/250/500/750/1.000 ms = 100/75/50/25/0 % (± 3 %). | T-FW-12 |
| LED-09 | Neuer Zug während Fade-out hat Vorrang. | Impuls während `fade` → Phase `pull`, Farbe startet wieder bei Grün. | T-FW-13 |
| LED-10 | Stromgrenze für LEDs (Schutz der Versorgung). | Firmware begrenzt den geschätzten LED-Strom auf `ledCurrentLimitMa` (D-05). | T-FW-14, T-HW-05 |

## 6. Sitzung, Spieler und Rangliste

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| SES-01 | Rangliste mit max. 5 Einträgen im RAM: Name, Dauer (ms), Sitzungszeitpunkt. | Nach 8 Zügen genau 5 Einträge; Rest verworfen. | T-FW-15 |
| SES-02 | Sortierung absteigend nach Dauer; bei Gleichstand gewinnt der früher abgeschlossene Zug. | Testfolge mit Gleichständen ergibt erwartete Reihenfolge. | T-FW-16 |
| SES-03 | Reset löscht die Rangliste, beendet keinen laufenden Zug. | Reset während Zug → Zug wird danach normal eingetragen. | T-FW-17 |
| SES-04 | Spielername max. 24 Zeichen (Unicode-Codepoints, UTF-8); leer → „Unbekannt“. | 24 Emoji/Umlaute werden akzeptiert, 25 abgelehnt (400); leer/Leerzeichen → „Unbekannt“. | T-API-08 |
| SES-05 | Name wird beim Zugbeginn übernommen; keine rückwirkende Änderung. | Namensänderung während Zug ändert den Eintrag dieses Zuges nicht. | T-FW-18 |
| SES-06 | Anzahl der erfassten Züge wird geführt. | `pullCount` entspricht der Zahl abgeschlossener Nicht-Diagnose-Züge seit Start/Reset (D-11). | T-FW-15 |
| SES-07 | Letzter abgeschlossener Zug wird bereitgestellt. | `session.lastPull` nach jedem Zug aktualisiert. | T-API-02 |

## 7. Webanwendung

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| WEB-01 | Access Point „Shisha-LED-Control“, offen, 192.168.4.1, HTTP Port 80. | Smartphone verbindet sich, `http://192.168.4.1/` lädt. | T-INT-01 |
| WEB-02 | Vollständig lokal, keine externen Ressourcen. | Netzwerk-Log des Browsers zeigt nur Anfragen an 192.168.4.1; Quelltext enthält keine `http(s)://` außer der eigenen IP in Hilfetexten. | T-UI-01 |
| WEB-03 | Acht Screens gemäß 05, erreichbar über die Navigation gemäß 07. | Jeder Screen ist von jedem anderen mit ≤ 2 Taps erreichbar. | T-UI-02 |
| WEB-04 | Responsiv, smartphonetauglich, Bedienelemente ≥ 44 × 44 px. | Darstellung ohne horizontales Scrollen bei 320–430 px Breite und im Querformat. | T-UI-03 |
| WEB-05 | Regelmäßige Statusaktualisierung (Start 250 ms, konfigurierbar im Client). | Laufende Zugdauer aktualisiert sich sichtbar mindestens 3× pro Sekunde. | T-UI-05 |
| WEB-06 | Globale UI-Zustände (Laden, Verbindung verloren, Fehler, Erfolg …) gemäß 05 §3. | Bei unterbrochener Verbindung wird spätestens nach 2 s „Verbindung verloren“ angezeigt und Live-Werte werden als veraltet markiert. | T-UI-06 |
| WEB-07 | Dunkles, kontrastreiches Design ohne externe Fonts/Bilder. | Kontrast Text/Hintergrund ≥ 4,5 : 1. | T-UI-07 |
| WEB-08 | Keine vorgetäuschten Systeminformationen. | Jeder angezeigte Wert stammt aus der API; nicht verfügbare Werte werden als „nicht verfügbar“ angezeigt. | T-UI-08 |
| WEB-09 | Bestätigungsdialog vor Leaderboard- und Konfigurations-Reset. | Reset ohne Bestätigung nicht auslösbar. | T-UI-09 |

## 8. HTTP-API

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| API-01 | Endpunkte `GET /`, `GET /api/status`, `GET /api/leaderboard`, `POST /api/config`, `POST /api/name`, `POST /api/leaderboard/reset`. | Alle Endpunkte antworten gemäß 06. | T-API-01…10 |
| API-02 | Strikte Validierung, alles-oder-nichts. | Ungültige Anfrage ändert keinen Zustand (Vorher/Nachher-Vergleich). | T-API-07 |
| API-03 | Begrenzte Request-Größe (≤ 512 Byte Body). | Größerer Body → 413, kein Absturz. | T-API-09 |
| API-04 | Definiertes Verhalten bei gleichzeitigen Anfragen. | 3 Clients mit je 4 Hz Polling + POSTs über 30 min ohne Fehler/Absturz. | T-LONG-01 |
| API-05 | Antworten zeigen nur echten Firmware-Zustand. | Code-Review: keine Platzhalter-Messwerte. | T-REV-02 |

## 9. Konfiguration und Standardwerte

| Parameter | Schlüssel | Standard | Bereich | Über API änderbar |
|---|---|---|---|---|
| Modus | `mode` | `smart` | `smart`, `steady`, `off` | ja |
| Zeit bis Gelb | `yellowMs` | 3.000 | 500–30.000, Raster 100 (D-09) | ja |
| Zeit bis Rot | `redMs` | 6.000 | `yellowMs`+100 … 60.000 (D-09) | ja |
| Abtastintervall | – | 10 ms | fest | nein (nur angezeigt) |
| Nachlaufzeit | `tailMs` | 1.000 | 200–5.000, Raster 50 (D-09) | ja |
| Fade-out | – | 1.000 ms | fest (D-10) | nein |
| Helligkeit | `brightness` | 64 | 0–255 | ja |
| Dauerlichtfarbe | `steadyColor` | Vorschlag (255, 160, 60) – **offen, D-07** | je Kanal 0–255 | ja |
| LED-Anzahl / Typ | – | 24 / WS2812 5050 (CR-01) | fest | nein |
| Spielername | via `/api/name` | „Unbekannt“ | 0–24 Codepoints | ja |
| Leaderboard-Größe | – | 5 | fest | nein |
| SSID / Passwort / IP | – | Shisha-LED-Control / keines / 192.168.4.1 | fest | nein |
| Diagnosemodus | `diagnostic` | `false` | bool | ja |
| Max. Zugdauer | – | Vorschlag 60.000 ms (D-08) | fest | nein |
| LED-Stromgrenze | – | Vorschlag 500 mA (D-05) | fest | nein |

## 10. Mechanik

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| MEC-01 | Aufnahme für das reale Glas (Auflagering Ø 109,1 mm, gemessen; ersetzt die Lastenheft-Annahme 150 mm) ohne Verspannung, mit seitlicher Führung; LED-Ring liegt in der Wölbung des Glasbodens und trägt kein Gewicht. | Testdruck nimmt das reale Glas ohne Klemmen auf; seitliches Spiel ≤ 1,5 mm; Glas kippelt nicht und berührt den LED-Ring nicht. | T-CAD-07 |
| MEC-02 | Parametrisches Modell mit zentralen Parametern (03 §3). | Änderung von `glass_contact_d` auf 100/120 mm erzeugt fehlerfrei ein angepasstes Modell. | T-CAD-02 |
| MEC-03 | Baugruppen: Grundkörper, LED-Träger, Elektronikaufnahme, Sensorhalter, Kabelwege, Servicezugang, Befestigung, Standfläche. | Alle Baugruppen im Modell und in der Stückliste vorhanden. | T-CAD-03 |
| MEC-04 | FDM-gerecht: keine geschlossenen Hohlräume, Überhänge ≤ 45° oder mit Strategie, Mindestwandstärke. | Mesh-Prüfung (manifold), Überhanganalyse, Slicer-Lauf ohne Fehler. | T-CAD-04…06 |
| MEC-05 | Elektronik und LEDs zerstörungsfrei austauschbar. | Demontage und Wiedermontage mit Standardwerkzeug ohne Beschädigung. | T-CAD-09 |
| MEC-06 | Schutz gegen verschüttete Flüssigkeit auf der Oberseite. | 50 ml Wasser in die Glasaufnahme gegossen → kein Wasser im Elektronikfach. | T-CAD-10 |
| MEC-07 | Rutschhemmende, stabile Standfläche. | Untersetzer mit befüllter Shisha verrutscht auf Glastisch bei leichtem Seitenstoß nicht; kein Kippeln. | T-CAD-11 |

## 11. Sicherheit und Dokumentation

| ID | Anforderung | Abnahmekriterium | Test |
|---|---|---|---|
| SAF-01 | Offener AP ohne Zugriffskontrolle ist als Einschränkung dokumentiert. | Hinweis in Hilfe-Screen und Bedienungsanleitung. | T-UI-10 |
| SAF-02 | Material wird nicht als hitzebeständig/flammhemmend angenommen. | Warnhinweis in Anleitung; kein Kontakt zu Kohle/Kopf vorgesehen. | T-REV-03 |
| SAF-03 | Keine freiliegenden Leiter; Zugentlastung. | Sichtprüfung nach Montage; 20-N-Zug am Kabel ohne Lösen der Lötstellen. | T-HW-08 |
| DOC-01 | Montage-, Verkabelungs-, Kalibrierungs- und Inbetriebnahmeanleitung (Phase B). | Dokumente 12–15 (siehe 11) vorhanden und von einer zweiten Person nachvollzogen. | T-DOC-01 |

## 12. Abgrenzung (nicht in V1)

Cloud, Fernzugriff, Benutzerkonten, Persistenz, Echtzeituhr, App-Store, Luftstrommessung, Personenerkennung, Akku, automatischer CAD-Export ohne Freigabe, Neustart-Funktion über die Weboberfläche (nur nach gesonderter Freigabe, D-14).

## 13. Abnahme der Planungsphase (Lastenheft §11)

| Nr. | Kriterium | Erfüllt durch | Status |
|---|---|---|---|
| 1 | Hardware und GPIOs dokumentiert | 02 | erledigt |
| 2 | Unsicherheiten Pinout/Sensorpegel benannt | 02 §2–3, 10 | erledigt |
| 3 | Mechanik berücksichtigt das Glas (Annahme 150 mm durch Messung Ø 109,1 mm ersetzt) | 03 | erledigt |
| 4 | Reale Maße identifiziert oder als Messaufgabe markiert | 03 §2, 10 (M-xx) | erledigt (Messaufgaben offen) |
| 5 | Parametrisches CAD-Konzept | 03 §3 | erledigt |
| 6 | SolidCraft-MCP untersucht und dokumentiert | 08 | erledigt: über den Rechner des Nutzers erreichbar, Befehlskatalog geprüft; Funktionstests in B5 |
| 7 | Firmware-Architektur | 04 | erledigt |
| 8 | Zustandsmaschine und Farbverlauf eindeutig | 04 §4–6 | erledigt |
| 9 | Acht Screens geplant | 05, 07 | erledigt |
| 10 | Navigation, UI-Zustände, Responsivität | 05 §2–4, 07 | erledigt |
| 11 | Endpunkte und JSON-Verträge | 06 | erledigt |
| 12 | RAM-Datenmodell und Standardwerte | 01 §9, 04 §3 | erledigt (D-07 offen) |
| 13 | Risiken bewertet | 10 §4 | erledigt |
| 14 | Testplan mit Abnahmekriterien | 09 | erledigt |
| 15 | Offene Entscheidungen dokumentiert | 10 | erledigt |
| 16 | Umsetzungsplan | 11 | erledigt |
| 17 | Dem Nutzer vorgelegt | – | mit Übergabe dieser Dokumente |
| 18 | Freigabe Phase B | – | **ausstehend** |
