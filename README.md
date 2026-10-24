# Smart Shisha LED-Untersetzer

3D-gedruckter LED-Untersetzer für eine Shisha: ESP32-C3 SuperMini, 24 × WS2812 LED-Ring, SW-420-Vibrationssensor zur Zugerkennung und lokale Web-App über einen eigenen WLAN-Access-Point (ohne Cloud).

## Inhalt

| Ordner | Inhalt |
|---|---|
| `docs/planung/` | Planungsdokumente 01–11 (Anforderungen, Hardware, Mechanik, Firmware, Web-App, API, Wireframes, CAD, Tests, offene Punkte, Umsetzungsplan) |
| `docs/planung/fotos/` | Fotos und Skizzen der realen Bauteile |
| `ShishaLed/` | Firmware (Arduino-Sketch). Stand B1: Kernlogik ohne Hardwareanbindung |
| `test/host/` | Host-Tests der Kernlogik (`make` im Ordner, braucht nur g++) |
| `test/b0_hardware/` | Testsketches für die Hardware-Verifikation (B0) |
| `cad/` | SolidCraft-Modell, Parameter, Ansichten, STL |

Einstieg: [`docs/planung/00_README.md`](docs/planung/00_README.md).

## Firmware bauen

- Arduino IDE 2.x mit Boardpaket „esp32“ von Espressif 3.x, Board „ESP32C3 Dev Module“, *USB CDC On Boot: Enabled*.
- Bibliothek „Adafruit NeoPixel“ (ab B2), „ArduinoJson“ 7 (ab B3). Die genauen Versionen werden beim ersten Gerätebuild festgehalten.
- Host-Tests: `cd test/host && make` (läuft auch automatisch bei jedem Pull Request).

## Status

Phase A (Planung) abgeschlossen, Phase B (Umsetzung) in Stufen freigegeben am 2026-10-24.
