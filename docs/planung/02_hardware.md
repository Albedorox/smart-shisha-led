# 02 – Hardware: Komponenten, Pinout, Stromversorgung

Stand: 2026-10-24 (Rev. 2, eingearbeitet: Pinout-Bild SuperMini, Fotos SW-420, LED-Ring mit 24 LEDs, Powerbank-Versorgung) · Phase A · Werte ohne Messung sind als **Annahme** markiert.

## 1. Komponentenliste (Elektronik)

| Pos. | Komponente | Menge | Status | Bemerkung |
|---|---|---|---|---|
| E1 | ESP32-C3 SuperMini | 1 | vorhanden, Pinout per Händlerbild belegt | 22,7 × 18,1 × 4,4 mm, Unterseite unbestückt |
| E2 | LED-Ring **24 × WS2812 5050** mit integriertem Treiber, Ø 86/71,8 mm, Pads DI/5V/GND/DO | 1 | vorhanden | **Abweichung vom Lastenheft (12 LEDs) → CR-01** |
| E3 | SW-420-Vibrationssensormodul, 31,2 × 14,2 mm, Pins VCC/GND/DO, LM393-Komparator, 10-kΩ-Poti, LEDs PWR und DO | 1 | vorhanden (Fotos) | |
| E4 | Pegelwandler 3,3 V → 5 V, 74AHCT125 (Breakout) oder 74AHCT1G125 | 1 | **Vorschlag**, D-03 | |
| E5 | Serienwiderstand Datenleitung 330 Ω | 1 | Vorschlag | nahe am Ring-DI |
| E6 | Elko 470–1000 µF / ≥ 6,3 V | 1 | Vorschlag | an den Ring-Pads 5V/GND |
| E7 | Keramikkondensator 100 nF | 1–2 | Vorschlag | Pegelwandler |
| E8 | USB-C-Einbaubuchse auf Breakout **mit 5,1-kΩ-Widerständen an CC1/CC2** | 1 | **Vorschlag**, D-06 | ohne CC-Widerstände liefern USB-C-zu-USB-C-Kabel/Powerbanks keine Spannung |
| E9 | Litze 0,25 mm² (5 V/GND), 0,14 mm² (Signale), Schrumpfschlauch | – | Vorschlag | Kabel direkt angelötet, keine Stiftleisten |
| E10 | Polyfuse 1,1 A (optional) im 5-V-Eingang | 1 | Vorschlag | Schutz bei Kurzschluss |
| E11 | Entfällt vorerst (D-29: Stufe 1). Optional: Keep-Alive-Last für Powerbanks (Logic-Level-MOSFET z. B. AO3400 + Lastwiderstand 33 Ω / 1 W) an GPIO 5 | 1 | **nur falls nötig**, D-29 | siehe §6 |
| E12 | Powerbank 5 V mit USB-A oder USB-C, ≥ 2 A empfohlen | 1 | vom Nutzer gewünscht | Lastenheft: kein eigenes Akkumanagement; die Powerbank bringt ihr BMS mit |

## 2. ESP32-C3 SuperMini – Pinout

Quelle: Pinout-Bild des Händlers (vom Nutzer am 2026-10-24 geliefert).

| Board-Pin | GPIO | Weitere Funktion laut Bild | Verwendung |
|---|---|---|---|
| 5V / G / 3.3 | – | Versorgung | 5 V Eingang, Masse, 3,3 V für Sensor |
| GPIO0 | 0 | ADC1-0 | **Sensor DO** (vorgesehen) |
| GPIO1 | 1 | ADC1-1 | frei |
| GPIO2 | 2 | A0/D0 | frei; Strapping-Pin des Chips → nicht verwenden |
| GPIO3 | 3 | A1/D1 | Reserve Sensor |
| GPIO4 | 4 | A2/D2 | **LED-Daten** (vorgesehen) |
| GPIO5 | 5 | A3/D3 | optional Keep-Alive-Last (D-29) |
| GPIO6/7 | 6/7 | SDA/SCL | frei |
| GPIO8 | 8 | SCK | Strapping-Pin; bei vielen SuperMini Onboard-LED → nicht verwenden |
| GPIO9 | 9 | MISO, BOOT-Taster | nicht verwenden |
| GPIO10 | 10 | MOSI | Reserve LED |
| GPIO20/21 | 20/21 | RX/TX | frei halten (Debug) |

**Bewertung:** GPIO 0 und GPIO 4 sind auf diesem Board herausgeführt und keine Strapping-Pins des ESP32-C3. Ein 32-kHz-Quarz an GPIO 0/1 ist auf dem Bild nicht zu erkennen. Status: **Pinout dokumentarisch bestätigt**; die elektrische Prüfung am Board folgt in T-HW-01 (Phase B, B0).

Noch offen (M-01b): Gibt es eine Diode zwischen USB-VBUS und dem 5V-Pin? Relevant, weil die Platine aus der Einbaubuchse über den 5V-Pin gespeist wird und beim Programmieren gleichzeitig USB am SuperMini steckt (Rückspeisung). Prüfung: Durchgangsmessung VBUS (USB-Buchse) ↔ 5V-Pin, oder Foto der Oberseite in hoher Auflösung. Bis zur Klärung gilt die Regel: **Zum Programmieren die Powerbank abziehen.**

| Weitere Eigenschaften (Annahme, aus Chip-Doku) | Konsequenz |
|---|---|
| Logikpegel 3,3 V, nicht 5-V-tolerant | Sensor an 3,3 V betreiben |
| Antenne an einer Schmalseite; bei manchen SuperMini-Klonen WLAN-Probleme, die durch reduzierte Sendeleistung (8,5 dBm) behoben werden | Antenne zur Außenwand; Sendeleistung als Konstante, T-HW-09 |

## 3. SW-420-Sensormodul

| Thema | Stand | Prüfung |
|---|---|---|
| Aufbau | LM393-Komparator, Poti (10 kΩ), Federkontakt, LEDs „PWR“ und „DO“ – auf den Fotos sichtbar | – |
| Versorgung | **an 3,3 V** (Vorschlag). Dann liegt DO sicher im 3,3-V-Bereich | T-HW-04 |
| Ausgangsstufe | LM393 Open-Collector mit Pull-up auf Modul-VCC (Annahme) | T-HW-04 |
| Polarität | Lastenheft: Ruhe HIGH, Erschütterung LOW. Je nach Modul und Poti-Einstellung wird auch die Umkehrung beobachtet → Kompilierkonstante `SENSOR_ACTIVE_LOW` | T-HW-04 |
| Impulsbreite | kurz und prellend (Annahme) → Interrupt-Latch (D-12) | T-HW-04 |
| Maße | 31,2 × 14,2 mm, M3-Loch 6,8 mm von der Stiftleisten-Kante | gemessen |

Hinweis: Der Sensor misst Erschütterungen, keinen Luftstrom (R-01). Ob das Blubbern durch Glas und Gehäuse zuverlässig erkannt wird, prüft B0 (R-02).

## 4. Verdrahtung (Prinzipschaltbild)

```
Powerbank ── USB-Kabel ──► USB-C-Einbaubuchse (CC1/CC2 je 5,1 kΩ nach GND)
                              │ VBUS                      │ GND
                         [Polyfuse 1,1 A]                 │
                              │                           │
             5V-Schiene ──────┼──────────────┬────────────┼──► LED-Ring 5V / GND (+ Elko 470–1000 µF an den Pads)
                              │              │            │
                     SuperMini 5V-Pin   74AHCT125 VCC     │   (+100 nF)
                                                          │
     SuperMini 3V3 ─────► SW-420 VCC                      │
     SuperMini GPIO 0 ◄── SW-420 DO                       │
     SuperMini GPIO 4 ──► 74AHCT125 A  (/OE = GND) ── Y ── 330 Ω ──► LED-Ring DI
     LED-Ring DO: nicht angeschlossen
     GND: gemeinsame Masse Buchse, SuperMini, SW-420, Pegelwandler, LED-Ring (sternförmig an der Buchse)

     optional (D-29): GPIO 5 ── 100 Ω ── Gate AO3400, Drain ── 33 Ω/1 W ── 5V-Schiene, Source ── GND, 100 kΩ Gate→GND
```

Der LED-Strom fließt direkt von der Eingangsbuchse zum Ring und nicht über Leiterbahnen des SuperMini (D-06).

## 5. Strombudget (24 LEDs)

| Verbraucher | Annahme | Strom |
|---|---|---|
| ESP32-C3, WLAN-AP aktiv | Mittel 80–120 mA, Spitzen bis ca. 350 mA | ≤ 350 mA Spitze |
| SW-420-Modul inkl. LEDs | | ≤ 10 mA |
| WS2812 5050, je LED und Kanal bei 255 | ca. 12–20 mA (Annahme 20 mA) | – |
| Alle 24 LEDs Weiß, 255 | 24 × 3 × 20 | **1.440 mA** |
| Smart-Modus, Gelb, Helligkeit 64 | 24 × 2 × 20 × 64/255 | ≈ **240 mA** |
| Ruhe im Smart-Modus (LEDs aus) | WS2812 Ruhestrom ca. 0,5–1 mA je LED (Annahme) | ≈ 12–24 mA |

Folgerungen:
- Ohne Begrenzung wären ca. 1,8 A möglich. Die Firmware begrenzt den geschätzten LED-Strom auf `LED_CURRENT_LIMIT_MA`, Vorschlag **500 mA** (D-05). Damit bleibt der Gesamtstrom bei ca. 0,85 A Spitze; jede Powerbank mit ≥ 1 A (besser 2 A) reicht.
- Die Helligkeitswirkung ist trotzdem höher als im Lastenheft geplant, weil 24 statt 12 LEDs leuchten.

## 6. Powerbank-Betrieb und Abschaltschutz

Wunsch des Nutzers: 3–4 Stunden je Session an einer Powerbank; die Powerbank darf bei geringer Last nicht abschalten.

**Laufzeit (Abschätzung):** ESP ca. 0,5 W + LEDs im Mittel 0,3 W (Smart) bis 2,5 W (Dauerlicht an der Stromgrenze). Für 4 h im ungünstigsten Fall ca. 12 Wh → ca. 3.500 mAh Nennkapazität einer Powerbank (3,7 V, 85 % Wirkungsgrad). Eine 10.000-mAh-Powerbank reicht für etwa drei Sessions im ungünstigsten und deutlich mehr im typischen Fall.

**Abschalten bei geringer Last:** Viele Powerbanks schalten ab, wenn der Strom längere Zeit unter einer Schwelle liegt (typisch 50–100 mA über 30–60 s; je nach Modell sehr unterschiedlich, Annahme). Der ESP32-C3 mit aktivem Access Point zieht bereits ca. 80–120 mA (Annahme). Wahrscheinlich hält das allein die meisten Powerbanks wach. Das muss mit der Powerbank des Nutzers gemessen werden (M-10, in B0).

Ein schwaches „Standby-Licht“ allein reicht dafür nicht: Eine LED auf niedriger Helligkeit zieht nur wenige mA. Es ist aber als sichtbare Bereitschaftsanzeige sinnvoll (D-30).

Gestufte Lösung (D-29):

| Stufe | Maßnahme | Wann |
|---|---|---|
| 1 | Nichts zusätzlich – Grundlast des ESP reicht | wenn M-10 zeigt, dass die Powerbank ≥ 30 min ohne LEDs anbleibt |
| 2 | Firmware-Keep-Alive über die LEDs: alle `KEEPALIVE_PERIOD_MS` (z. B. 10 s) für 150 ms ein kurzer, schwacher Lichtpuls | einfache Lösung ohne Zusatzteile; sichtbar, kann als „Atmen“ gestaltet werden |
| 3 | Unsichtbare Pulslast: MOSFET + 33 Ω (≈ 150 mA) für 150–300 ms alle 10 s (E11) → Mittelwert ca. 3–5 mA | wenn die Powerbank einen Mindeststrom > Grundlast verlangt und kein Licht gewünscht ist |

Die Pulsdauer und Periode sind Konstanten, weil die Schwellen je Powerbank verschieden sind. Das Lastenheft schließt eigenes Akkumanagement aus; die Keep-Alive-Funktion ist daher eine **neue Anforderung (CR-02)** und braucht die Freigabe des Nutzers.

## 7. Datenleitung und Pegel (D-03)

WS2812 erwartet laut gängigen Datenblättern V_IH ≥ 0,7 × VDD, bei 5 V also ≥ 3,5 V. Ein 3,3-V-GPIO liegt darunter und funktioniert nicht zuverlässig über Temperatur und Exemplare (R-05). **Vorschlag: 74AHCT125** (als fertiges Breakout erhältlich). Bidirektionale MOSFET-Pegelwandler (BSS138) sind für 800-kHz-Daten nur bedingt geeignet.

## 8. Thermik

- LEDs bei Stromgrenze 500 mA: ca. 2,5 W, verteilt über 24 LEDs auf Ø 86 mm, direkt unter dem Glasboden.
- Ziel T-HW-07: LED-Ring ≤ 60 °C, Gehäuse ≤ 40 °C nach 60 min Dauerlicht.

## 9. Elektrische Schutzmaßnahmen

- Lötstellen mit Schrumpfschlauch isoliert; keine blanken Leiter.
- Polyfuse im Eingang (E10).
- Zugentlastung am USB-Eingang über die verschraubte Einbaubuchse.
- Nur 5-V-USB-Quellen (Powerbank oder Netzteil); keine Schnellladeprotokolle nötig (die CC-Widerstände fordern nur 5 V an).

## 10. Messaufgaben Hardware

| ID | Aufgabe | Status |
|---|---|---|
| M-01 | Board-Revision und Pinout | **erledigt** (Händler-Pinout) |
| M-01b | Diode VBUS ↔ 5V-Pin vorhanden? | offen |
| M-02 | SuperMini-Maße | **erledigt** |
| M-03 | LED-Ring-Maße und Padlage | **erledigt**; M-03b (Höhen) offen |
| M-04 | SW-420-Maße | **erledigt**; Polarität und Impulsbreite in B0 (T-HW-04) |
| M-05 | Stromaufnahme realer Aufbau | in B0/B2 |
| M-07b | gewählte USB-C-Einbaubuchse | nach Kauf |
| M-10 | Powerbank des Nutzers: bleibt sie bei ca. 100 mA dauerhaft an? Abschaltzeit ohne Last? | in B0 |
