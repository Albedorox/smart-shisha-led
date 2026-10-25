# Smart Shisha LED – Anleitung Rohfassung (Stand 2026-10-24)

Die Rohfassung ist vollständig, aber noch nicht am Gerät erprobt. Offene Punkte sind als Annahmen gesetzt (Liste unten) und werden nachträglich angepasst.

## 1. Drucken

| Teil | Datei | Lage auf dem Bett |
|---|---|---|
| T1 Deckplatte | `cad/v2/stl/T1_Deckplatte_v2.stl` | Glasauflage nach oben |
| T2 Unterschale | `cad/v1/stl/T2_Unterschale_v1.stl` | Boden auf dem Bett |
| T4 Distanzring | `cad/v1/stl/T4_Distanzring_v1.stl` | flach |

PETG oder PLA, ohne Stützmaterial. Druckeinstellungen: `docs/planung/03_mechanical_design.md` §13.

## 2. Teile

ESP32-C3 SuperMini, LED-Ring 24 × WS2812, SW-420, rotes USB-C-Breakout (5,1 kΩ an CC), Elko 470–1000 µF / ≥ 6,3 V, Litze, 4 × M2-Einschmelzmutter + 4 × M2-Schraube (Ring), 5 × M3-Einschmelzmutter + 4 × M3-Schraube (Gehäuse) + 1 × M3-Schraube (Sensor), 2 × M2,5-Schraube selbstschneidend ca. 6 mm (Breakout), 4 Silikonfüße Ø10.

## 3. Verdrahtung

| Von | Nach |
|---|---|
| Breakout VBUS | SuperMini 5V, LED-Ring 5V |
| Breakout GND | SuperMini GND, LED-Ring GND, SW-420 GND |
| SuperMini GPIO4 | LED-Ring DI (Annahme: ohne Pegelwandler; flackert der Ring, 74AHCT125 nachrüsten) |
| SuperMini 3V3 | SW-420 VCC |
| SuperMini GPIO0 | SW-420 DO |
| Elko | direkt an den Ring-Pads 5V/GND (Polung beachten) |

Kabel direkt anlöten, Lötstellen mit Schrumpfschlauch isolieren.

## 4. Montage

1. Einschmelzmuttern mit dem Lötkolben setzen: 4 × M2 und 5 × M3 in T1 (von unten).
2. Kabel an die Ring-Pads löten, durch die rechteckige Aussparung führen, Ring mit 4 × M2 verschrauben. Aussparung mit Heißkleber oder Silikon abdichten.
3. SW-420 mit T4 als Distanzring und M3-Schraube an T1 schrauben; die zwei Stifte in T2 sichern ihn gegen Verdrehen.
4. SuperMini in den Rahmen in T2 legen (USB zur Programmieröffnung), mit Heißkleber oder Schaumklebeband fixieren.
5. Breakout mit 2 × M2,5 auf die Sockel schrauben, Platinenkante in der Wandtasche.
6. T1 auf T2 setzen (Feder in Nut), mit 4 × M3 von unten verschrauben, Silikonfüße einkleben.

## 5. Firmware aufspielen

Am einfachsten ohne Arduino IDE direkt im Browser: siehe [flashen.md](flashen.md). Alternativ mit der Arduino IDE:

1. Arduino IDE mit ESP32-Core (Espressif) und der Bibliothek „Adafruit NeoPixel“.
2. Board „ESP32C3 Dev Module“, „USB CDC On Boot: Enabled“.
3. `ShishaLed/ShishaLed.ino` öffnen und hochladen (Powerbank dabei abziehen).
4. Empfindlichkeit am Poti des SW-420 einstellen: Im seriellen Monitor (115200 Baud) erscheint jeder erkannte Zug mit Dauer.

## 6. Bedienung

1. Powerbank anschließen. Das Gerät öffnet das offene WLAN **ShishaLed**.
2. Mit dem Handy verbinden und **http://192.168.4.1** öffnen.
3. In der Web-App: Name eingeben, Zugzeit und Top 5 ansehen, Modus (Smart / Dauerlicht / Aus), Helligkeit, Farbe, Verlauf und Nachlauf einstellen. Einstellungen bleiben nach dem Ausschalten erhalten; die Rangliste nicht.

## Annahmen (nachträglich prüfen)

- Sensorpolarität: aktiv bei LOW (`SENSOR_ACTIVE_LOW`), B0 noch offen
- Einschmelzmuttern: Bohrungen M2 Ø3,2 / M3 Ø4,0 (Coupon nicht getestet)
- Testring v2 (Zentrierrand, Ringtasche, Aussparung) nicht gedruckt
- Glasabstand zu den LEDs rechnerisch ca. 1,1 mm
- LED-Strom 20 mA je Kanal, Stromgrenze 500 mA
- Breakout-Maße laut Datenblatt, nicht nachgemessen
- Datenleitung ohne Pegelwandler
