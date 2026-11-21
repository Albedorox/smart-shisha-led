# B0 – Hardware-Testsketches

Wegwerfcode für die Hardware-Verifikation (Plan: `docs/planung/11_implementation_plan.md`, Paket B0). Nicht Teil der Firmware.

## Arduino IDE einrichten

| Einstellung | Wert |
|---|---|
| Boardpaket | „esp32“ von Espressif, Version 3.x (Boardverwalter-URL `https://espressif.github.io/arduino-esp32/package_esp32_index.json`) |
| Board | „ESP32C3 Dev Module“ (oder „Nologo ESP32C3 Super Mini“, falls vorhanden) |
| USB CDC On Boot | **Enabled** (sonst bleibt der serielle Monitor leer) |
| Bibliothek | „Adafruit NeoPixel“ (Bibliotheksverwalter) |
| Serieller Monitor | 115200 Baud |

Falls der Upload nicht startet: BOOT-Taster gedrückt halten, kurz RESET drücken, BOOT loslassen, erneut hochladen.

## Aufbau für den Test

```
SuperMini 3V3  ── SW-420 VCC
SuperMini GND  ── SW-420 GND ── LED-Ring GND ── Pegelwandler GND
SuperMini GPIO0 ◄─ SW-420 DO
SuperMini GPIO4 ─► 74AHCT125 A (/OE an GND, VCC an 5V) ─ Y ─ 330 Ω ─► LED-Ring DI
SuperMini 5V   ── LED-Ring 5V ── 74AHCT125 VCC   (+ 470–1000 µF zwischen 5V und GND am Ring)
```

Ohne Pegelwandler funktioniert der Ring für einen kurzen Tisch-Test oft trotzdem (GPIO4 → 330 Ω → DI). Für den Dauerbetrieb ist er vorgesehen (02 §7).

## Ablauf

1. **B0_SensorTest**: SW-420 an GPIO0, Ring nicht nötig.
   - Nach dem Start 3 s nicht berühren. Der Sketch meldet den Ruhepegel und den Wert für `SENSOR_ACTIVE_LOW`.
   - Dann am Tisch klopfen. Danach den Sensor so unter das Glas legen, wie er später sitzt, und ein paar echte Züge machen.
   - Jeder Zug erscheint als „Zug #n Start / Ende: Dauer …“. Mit `s` die 1-s-Statistik, mit `p` die Plotter-Ausgabe (Werkzeuge → Serieller Plotter).
   - Poti am SW-420 so einstellen, dass Züge sicher erkannt werden, aber Abstellen des Glases oder Tischklopfen möglichst nicht.
2. **B0_LedRingTest**: Ring an GPIO4, Sensor nicht nötig.
   - Startet mit dem Einzeltest (jede LED Rot/Grün/Blau, Nummer im Monitor), danach automatisch die Zug-Demo.
   - Ring unter das Glas halten und die Lichtwirkung beurteilen (`+`/`-` für die Helligkeit).
   - `3` für die Strommessung mit USB-Messgerät (Weiß 32/64/128).
3. **B0_ZugUndLicht**: beides angeschlossen. Oben im Sketch `SENSOR_ACTIVE_LOW` aus Schritt 1 eintragen.
   - Echte Session: Leuchtet der Ring beim Ziehen, wechselt er nach 3 s auf Gelb und nach 6 s auf Rot, blendet er nach dem Zug aus?
   - Powerbank anschließen und 10 min im Ruhezustand stehen lassen. Bleibt sie an (Keep-Alive, M-10)?

## Messprotokoll (bitte ausfüllen und zurückgeben)

| Nr. | Prüfung | Ergebnis |
|---|---|---|
| T-HW-01 | Board startet mit angeschlossenem Sensor normal, GPIO0/GPIO4 funktionieren | |
| T-HW-04 | Ruhepegel / `SENSOR_ACTIVE_LOW` | Ruhe LOW, Vibration HIGH → `false` (2026-11-08) |
| T-HW-04 | kürzeste / längste Impulsbreite (µs) beim Ziehen | |
| R-02 | Züge am Glas erkannt? (z. B. 10 Züge: wie viele erkannt) | |
| R-02 | Fehlauslösungen beim Abstellen des Glases / Klopfen auf den Tisch | |
| R-02 | gemessene Zugdauer passt zur gefühlten (Stoppuhr) | |
| T-HW-06 | alle 24 LEDs in allen Farben, LED 0 = am DI-Pad | |
| R-14 | Lichtwirkung durchs Glas bei Helligkeit 64 (zu hell / gut / zu schwach) | |
| T-HW-05 | Strom Leerlauf / Weiß 32 / 64 / 128 (mA) | |
| M-10 | Powerbank bleibt im Ruhezustand an (Modell, Dauer) | |
| – | Poti-Stellung, die am besten funktioniert hat (Foto genügt) | |

## B0_Beschleunigung: LIS2DW12 statt SW-420

Prüft, ob der Beschleunigungssensor LIS2DW12 das Blubbern beim Ziehen durch Glas und Deckplatte erkennt. Hintergrund und Rechnung: `docs/planung/12_vibrationssensor_bewertung.md` (§7). Der SW-420 darf angeschlossen bleiben, er läuft zum Vergleich mit.

### Anschluss

| LIS2DW12-Modul | SuperMini | Bemerkung |
|---|---|---|
| VCC | 3V3 | Modul hat einen eigenen Regler, 3,3 V reichen |
| GND | GND | |
| SDA | GPIO6 | Pull-ups 10 kΩ sind auf dem Modul |
| SCL | GPIO7 | |
| CS, SDO, IT1, IT2 | offen lassen | SDO offen → I²C-Adresse 0x19 (an GND: 0x18, wird auch gefunden) |

LED-Ring (GPIO4) und SW-420 (GPIO0) bleiben wie in der Firmware.

### Befestigen

- Unterseite der Deckplatte, **direkt unter dem Auflagering des Glases** (Ø ca. 109 mm), Bauteilseite nach unten.
- **Flach** aufkleben, damit die Z-Achse senkrecht steht. Der Sketch prüft das und meldet es.
- **Dünnes, hartes** doppelseitiges Klebeband (Folienband) oder ein Tropfen Sekundenkleber/Heißkleber. **Kein Schaumklebeband**, das federt die Vibration weg.
- Kabel lose verlegen, sie dürfen nicht am Modul ziehen.

### Flashen

Wie in `docs/flashen.md`, nur mit einer anderen Datei:

- aus dem Pull Request: **Actions** → Lauf „Testsketch Beschleunigung“ → **Artifacts** → `B0_Beschleunigung-firmware` (ZIP entpacken), oder
- nach dem Merge: **Releases** → **test-beschleunigung** → `B0_Beschleunigung-merged.bin`.

Adresse `0x0`. Gespeicherte Einstellungen der Firmware bleiben erhalten. **Nach dem Test wieder `firmware-latest` flashen.**

Alternativ in der Arduino IDE öffnen und hochladen (Einstellungen oben, Bibliothek „Adafruit NeoPixel“).

### Was der LED-Ring zeigt

| Anzeige | Bedeutung |
|---|---|
| ein weißer Punkt läuft im Kreis | Ruhemessung (10 s): nichts berühren, nicht ziehen |
| grüner Balken | Vibrationspegel unter der Schwelle (8 LEDs = doppelt so stark wie Ruhe) |
| blauer Balken | Pegel über der Schwelle, das wäre ein erkannter Zug |
| eine schwach rote LED | Lage der Schwelle (Start: 2-fache Ruhe) |
| ganzer Ring blinkt rot | kein Sensor gefunden: Verdrahtung prüfen |
| ganzer Ring blinkt orange | falscher Chip (z. B. LIS2DS12): Modul zurückschicken |

Der Ring funktioniert auch ohne Computer an der Powerbank: Leuchtet beim Ziehen ein blauer Balken auf, der beim Abstellen des Glases oder Anfassen des Schlauchs nicht kommt, ist das schon ein gutes Zeichen.

### Ablauf mit seriellem Monitor (115200 Baud)

1. Nach dem Start prüft der Sketch die Chip-Kennung: **`WHO_AM_I = 0x44 -> LIS2DW12 erkannt`** muss erscheinen.
2. 10 s Ruhemessung. Danach meldet er den Ruhepegel, das zu erwartende Sensorrauschen (ca. 1 mg auf Z) und ob der Sensor flach liegt.
3. Ab dann kommt jede Sekunde eine Zeile mit dem Pegel. Ein Zug wird als `Zug #n: Dauer, Pegel, Hauptfrequenz` gemeldet, Klopfer als `kurzer Stoß`.
4. Abschnitte markieren (Taste + Enter):
   - `1` direkt vor einem Zug, `0` direkt danach. **5 Züge**, normal stark.
   - `2` und dann den Schlauch anfassen und bewegen, ohne zu ziehen, danach `0`.
   - `3` und dann zwei-, dreimal auf den Tisch tippen, danach `0`.
   - zwischendurch einmal `f` während eines Zugs: zeigt das Spektrum (bei welcher Frequenz es blubbert).
5. `e` zeigt die **Auswertung**: Pegel je Abschnitt, Faktor zur Ruhe, Anteil über der Schwelle, SW-420-Flanken zum Vergleich und ein Ergebnis:
   - Zug ≥ 3-mal Ruhe: reicht mit Reserve
   - 2- bis 3-mal: reicht knapp
   - unter 2-mal: Montage verbessern, sonst stärkerer Sensor
6. Bitte die komplette Ausgabe kopieren und zurückgeben, mindestens die Auswertung.

Weitere Tasten: `+`/`-` Schwelle, `a` Z-Achse oder alle Achsen, `k` Ruhe neu messen, `p` Plotter (Werkzeuge → Serieller Plotter), `r` Rohdaten mit 400 Hz, `q` nur Ereignisse, `h` Hilfe.

### Messprotokoll LIS2DW12

| Nr. | Prüfung | Ergebnis |
|---|---|---|
| S-01 | WHO_AM_I (0x44 erwartet), I²C-Adresse | |
| S-02 | Ruhepegel Z (mg) und erwartetes Sensorrauschen laut Sketch | |
| S-03 | Lage: liegt flach (Z ≈ ±1000 mg)? | |
| S-04 | Faktor Zug/Ruhe aus `e` | |
| S-05 | 5 Züge: wie viele als `Zug #n` erkannt? | |
| S-06 | Hauptfrequenz beim Ziehen | |
| S-07 | Schlauch anfassen / Tisch tippen: als Zug erkannt? | |
| S-08 | SW-420-Flanken bei Zug / Schlauch / Tisch (Vergleich) | |
| S-09 | Befestigung (Klebeband, Kleber) und Ort | |
