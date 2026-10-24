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
| T-HW-04 | Ruhepegel / `SENSOR_ACTIVE_LOW` | |
| T-HW-04 | kürzeste / längste Impulsbreite (µs) beim Ziehen | |
| R-02 | Züge am Glas erkannt? (z. B. 10 Züge: wie viele erkannt) | |
| R-02 | Fehlauslösungen beim Abstellen des Glases / Klopfen auf den Tisch | |
| R-02 | gemessene Zugdauer passt zur gefühlten (Stoppuhr) | |
| T-HW-06 | alle 24 LEDs in allen Farben, LED 0 = am DI-Pad | |
| R-14 | Lichtwirkung durchs Glas bei Helligkeit 64 (zu hell / gut / zu schwach) | |
| T-HW-05 | Strom Leerlauf / Weiß 32 / 64 / 128 (mA) | |
| M-10 | Powerbank bleibt im Ruhezustand an (Modell, Dauer) | |
| – | Poti-Stellung, die am besten funktioniert hat (Foto genügt) | |
