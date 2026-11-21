# 12 – Bewertung Vibrationssensor für die Blubber-Erkennung

Stand: 2026-11-21 · Abschätzung, keine Messung · Entscheidung: LIS2DW12 wird getestet (§7) · alle Annahmen sind als **(A)** markiert

## Kurzfazit

1. **Der SW-420 reicht mit hoher Wahrscheinlichkeit nicht.** Das Blubbern erzeugt am Untersetzer nach dieser Abschätzung etwa **0,3 bis 10 mg** (Bestwert 1 bis 2 mg, 1 mg = 0,001 g) bei **20 bis 120 Hz**. Ein Federkontakt wie der SW-420 braucht nach meiner Schätzung **einige hundert mg** zum Öffnen. Das ist ein Faktor 20 (günstigster Fall) bis 1.000 zu wenig. Er wird Klopfen, Anstoßen und das Hantieren mit dem Schlauch melden, aber nicht das Blubbern selbst.
2. **Ein Beschleunigungssensor ist der richtige Weg, aber nicht jeder.** Das Signal liegt in der Größenordnung des Rauschens günstiger MEMS-Sensoren. Der **ADXL345 ist zu verrauscht** (etwa 3 bis 6 mg rms). Der **LIS3DH ist grenzwertig** (etwa 2 mg rms im Nutzband, mit Auswertung über 0,5 s erkennbar ab etwa 1,5 mg). Mit Reserve arbeitet ein rauscharmer Sensor wie der **LIS2DW12** (etwa 4-mal besser) oder der **ADXL355** (etwa 10-mal besser, teurer).
3. **Piezo** ist billig und sehr empfindlich, misst aber vor allem Kraft- und Biegeänderungen und hängt stark von der Montage ab. Er braucht eine kleine Verstärkerstufe und reagiert genauso auf Klopfen und Schall.
4. **Die Unsicherheit ist groß** (etwa Faktor 10 in beide Richtungen beim Signal). Sie ändert das Urteil über den SW-420 nicht, wohl aber die Wahl zwischen LIS3DH und rauschärmeren Sensoren. Deshalb: vor der Festlegung einen kurzen Messtest mit dem Beschleunigungssensor machen (§7).

## 1. SW-420: Mechanik und Auslöseschwelle

**Was die Quellen sagen:**
- Das Bauteil ist ein **Federschalter, in Ruhe geschlossen** (Normally Closed): eine Feder umgibt einen Stift und berührt ihn. Bei Erschütterung hebt die Feder kurz ab, der Kontakt öffnet für Millisekunden. [Handsontec](https://handsontec.com/dataspecs/sensor/Vibration%20Sensor.pdf), [Rajguru](https://5.imimg.com/data5/DV/NE/MY-1833510/vibration-sensor.pdf), [SunFounder](https://docs.sunfounder.com/projects/ultimate-sensor-kit/en/latest/components_basic/04-component_vibration.html)
- Das Modul vergleicht mit einem **LM393-Komparator** die Spannung am Schalter mit einer Poti-Spannung. Ist der Schalter zu, liegt der Eingang auf 0 V; öffnet er, wird er über einen Widerstand hochgezogen. [Seeed](https://wiki.seeedstudio.com/Grove-Vibration_Sensor_SW-420/)
- Kontaktprellen etwa 1 bis 10 ms, empfindlich quer zur Federachse, kann durch Schall auf schwingenden Flächen auslösen. [Tech-Blog SW-420/Piezo](https://tech-blog-31y.pages.dev//vibration)
- **Keine einzige Quelle nennt eine Auslöseschwelle in g oder einen Frequenzbereich.** Alle Beispiele arbeiten mit Klopfen, Schütteln oder Anstoßen.
- Die Polarität von DO ist je nach Quelle unterschiedlich (deckt sich mit 02_hardware §3, `SENSOR_ACTIVE_LOW`).

**Wichtige Folgerung zum Poti:** Das Poti verstellt nur die Komparatorschwelle, also wie lange bzw. wie weit der Kontakt öffnen muss, damit das RC-Glied die Schwelle erreicht. **Die mechanische Empfindlichkeit der Feder ändert es nicht.** Wenn die Feder nicht abhebt, hilft keine Poti-Stellung.

**Eigene Schätzung der Schwelle (A):** Damit die Feder abhebt, muss die Beschleunigung die Vorspannkraft der Feder am Stift relativ zu ihrer Masse übersteigen. Bei Bauform und Federstärke solcher Schalter liegt das nach meiner Einschätzung bei **etwa 0,2 bis 1 g (200 bis 1.000 mg)**, mit Streuung von Exemplar zu Exemplar. Diese Zahl ist **nicht durch ein Datenblatt belegt**. Sie passt aber zum Verhalten aus allen Beispielen (reagiert auf Klopfen, nicht auf leises Brummen).

## 2. Abschätzung: Was das Blubbern am Untersetzer erzeugt

### Annahmen

| Größe | Annahme (A) | Bemerkung |
|---|---|---|
| Gesamtmasse Shisha auf dem Untersetzer | 3 bis 6 kg, Nennwert 4 kg | Glasbowl 1,5–2,5 kg, Wasser 0,5–1 kg, Rauchsäule und Kopf 1–2 kg |
| Luftstrom beim Zug | 0,2 bis 0,5 l/s | Zug 2–4 s, 0,5–1,5 l |
| Blasendurchmesser am Tauchrohr | 15 bis 25 mm (1,8 bis 8,2 cm³) | ohne Diffusor; mit Diffusor viele kleine Blasen, breitbandiger und höherfrequent |
| Wechselkraft je Blase | etwa Auftriebskraft ρ·g·V = 0,02 bis 0,08 N; kräftiger Zug mit Schwappen bis 0,2 N | Nennwert 0,05 N |
| Kopplung Shisha–Untersetzer–Tisch | Untersetzer und Shisha bewegen sich gemeinsam auf der Tischnachgiebigkeit, Steifigkeit 10⁵ bis 10⁶ N/m | ergibt Eigenfrequenz 25 bis 80 Hz |
| Dämpfung Glas/PLA | im Bereich unter 200 Hz gering, Glas und Deckplatte koppeln praktisch starr | höhere Frequenzen werden durch PLA und den Auflagering gedämpft |

### Rechnung

**Frequenz:** Blasenfolge f = Luftstrom / Blasenvolumen = 200 cm³/s / 8,2 cm³ ≈ **24 Hz** bis 500 cm³/s / 1,8 cm³ ≈ **280 Hz**. Typisch für das hörbare Blubbern ohne Diffusor: **20 bis 120 Hz**.

**Wechselkraft:** Steigt eine Blase auf, bewegt sich die gleiche Wassermenge nach unten. Die äußere Kraft auf den Untersetzer schwankt dabei in der Größenordnung der Auftriebskraft der Blase: **0,02 bis 0,2 N**, Nennwert 0,05 N. Druckschwankungen im Gefäß sind innere Kräfte und heben sich nach außen weitgehend auf.

**Beschleunigung am Untersetzer:** Oberhalb der Tisch-Eigenfrequenz gilt a ≈ F / M:

| Fall | F | M | a |
|---|---|---|---|
| schwach | 0,02 N | 6 kg | **0,3 mg** |
| Nennwert | 0,05 N | 4 kg | **1,3 mg** |
| kräftig | 0,2 N | 3 kg | **6,8 mg** |

Unterhalb der Eigenfrequenz wird es weniger. Trifft die Blasenfrequenz genau die Eigenfrequenz des Systems auf dem Tisch, kann das Signal um den Faktor 3 bis 10 überhöht werden, also kurzzeitig 10 mg und mehr. Lokal am Glasboden gibt es zusätzlich höherfrequente Anteile (Platzen der Blasen, Schwappen an der Wand), die aber durch PLA und den 2 mm schmalen Auflagering gedämpft werden.

**Ergebnis:** etwa **0,3 bis 10 mg**, wahrscheinlich **1 bis 2 mg**, Hauptanteil **20 bis 120 Hz**, als Folge von Einzelstößen während des Zugs. Unsicherheit: etwa Faktor 10 in beide Richtungen.

**Zum Vergleich (Störquellen):** Schlauch anfassen oder ziehen, Kohle nachlegen, Glas abstellen oder an den Tisch stoßen erzeugen eher **50 bis 1.000 mg**. Das sind genau die Ereignisse, auf die der SW-420 reagiert. Ein Test „SW-420 hat beim Rauchen ausgelöst“ beweist also noch nicht, dass er das Blubbern erkennt.

## 3. Vergleich der Sensoren

Rauschen im Nutzband 20 bis 120 Hz (Bandbreite etwa 100 Hz): Rauschen rms = Rauschdichte × √Bandbreite ([Formel laut ST](https://community.st.com/t5/mems-sensors/accelerometers-comparison/td-p/770981)). Erkennungsgrenze: Mit einer Energie-Auswertung im Band über 0,5 s lässt sich ein Signal ab etwa 0,6- bis 0,7-fachem Rauschwert sicher erkennen (A, statistische Faustregel).

| Sensor | Prinzip | Auflösung / Rauschen | Rauschen im Nutzband | Erkennbar ab etwa | Verhältnis zum Blubber-Signal (1–2 mg) | Preis (A) |
|---|---|---|---|---|---|---|
| **SW-420** | Federkontakt, digital | Schwelle mechanisch, geschätzt 200–1.000 mg (kein Datenblatt) | – | **200–1.000 mg** | **100- bis 1.000-mal zu schwach** | < 1 € |
| **ADXL345** | MEMS, I²C, 13 bit | 3,9 mg/LSB; 0,75 LSB rms (X/Y) und 1,1 LSB rms (Z) bei 100 Hz ODR ([Datenblatt](https://people.ece.ubc.ca/edc/7660.jan2018/ADXL345.pdf)) ≈ 410 bzw. 600 µg/√Hz | 4 bis 6 mg | **3–4 mg** | zu schwach, nur bei kräftigem Zug | 2–3 € (GY-291) |
| **LIS3DH** | MEMS, I²C, 12 bit High-Res | 220 µg/√Hz ([ST Community](https://community.st.com/t5/mems-sensors/lis3dh-noise-characteristics/m-p/542087)) | 2,2 mg | **≈ 1,5 mg** | **grenzwertig**, etwa 50/50 | 2–5 € |
| **LIS2DW12** | MEMS, I²C, Low-Noise-Modus | 90 µg/√Hz ([ST Community](https://community.st.com/t5/mems-sensors/accelerometers-comparison/td-p/770981)) | 0,9 mg | **≈ 0,6 mg** | **ausreichend** im Nennfall | 3–8 € (Breakout, Verfügbarkeit prüfen) |
| **ADXL355** | MEMS, I²C/SPI, 20 bit | ca. 25 µg/√Hz (aus dem Gedächtnis, im Datenblatt prüfen) | 0,25 mg | **≈ 0,2 mg** | **klare Reserve** | 30–50 € (Eval-Board) |
| **Piezo-Scheibe** (27 mm) | Ladung bei Biegung/Kraft, analog | Empfindlichkeit in Biegung grob 0,1 bis 5 V/N (A, montageabhängig) | ADC-Rauschen ESP32-C3 einige mV | bei 0,05 N ca. 5–250 mV | **wahrscheinlich ausreichend**, mit Verstärker sicher | < 1 € + Op-Amp |

Zum Vergleich der Energie: Der ESP32-C3 hat I²C an GPIO 6/7 frei (02_hardware §2). Alle MEMS-Sensoren brauchen unter 0,2 mA und passen in das Strombudget.

### Bewertung im Einzelnen

**SW-420:** Fällt aus. Auch im günstigsten Fall (10 mg Signal, 200 mg Schwelle) fehlt ein Faktor 20. Möglich ist nur, dass ein einzelnes, besonders „lockeres“ Exemplar knapp in Ruhe flattert; das wäre Zufall und nicht reproduzierbar.

**ADXL345:** Weit verbreitet, aber das Rauschen liegt über dem erwarteten Signal. Höchstens als Notlösung bei kräftigem Zug.

**LIS3DH:** Gut verfügbar, gute Bibliotheken (Adafruit), FIFO und Hochpass-Filter im Chip. Bei 400 Hz ODR und Auswertung der Bandenergie 20 bis 120 Hz über 0,5 s liegt er genau an der Grenze des Nennfalls. Bei dicken Blasen und kräftigem Zug funktioniert er, bei schwachem Zug mit Diffusor wahrscheinlich nicht.

**LIS2DW12:** Bester Kompromiss aus Preis und Reserve. Etwa 2,5-mal rauschärmer als der LIS3DH, ebenfalls mit FIFO und Low-Power-Modi.

**ADXL355:** Technisch die sicherste Lösung, aber für einen Bausatz teuer und als fertiges Modul seltener.

**Piezo:** Kann sehr empfindlich sein, wenn er im Kraftfluss oder auf einer dünnen, biegenden Fläche direkt unter dem Auflagering sitzt. Nachteile: Signal hängt stark von Kleber, Lage und Vorspannung ab (schwer reproduzierbar für einen Bausatz), braucht Vorspannung auf halbe ADC-Spannung plus Verstärkung 10 bis 50, und Klopfen, Schall und Musik landen genauso im Signal.

## 4. Unsicherheiten

| Unsicherheit | Wirkung auf das Ergebnis |
|---|---|
| Wechselkraft je Blase (Modell „Auftriebskraft“) | Faktor 3 in beide Richtungen |
| Tisch und Kopplung (Holz, Glas, Teppich, Füße) | Resonanzüberhöhung 1- bis 10-fach oder Dämpfung |
| Diffusor ja/nein, Wasserstand, Zugstärke | verschiebt Frequenz (20 Hz bis über 200 Hz) und Amplitude |
| SW-420-Schwelle | keine Datenblattangabe, eigene Schätzung 200 bis 1.000 mg |
| Piezo-Empfindlichkeit | stark montageabhängig, Faktor 50 Spanne |
| Rauschdichte LIS3DH | aus Community-Beitrag, der den Datenblattwert zitiert; ADXL355-Wert aus dem Gedächtnis |

Das Urteil zum SW-420 ist robust gegenüber diesen Unsicherheiten. Ob der LIS3DH reicht, hängt dagegen davon ab, wo im Bereich 0,3 bis 10 mg das echte Signal liegt.

## 5. Empfehlung

1. **SW-420 nicht weiterverfolgen** für die Blubber-Erkennung. Er kann höchstens als „Shisha wird bewegt“-Melder dienen.
2. **Beschleunigungssensor, aber rauscharm:** Zielwert **≤ 100 µg/√Hz**. Empfehlung **LIS2DW12**. Als günstigen Ersttest kann man den **LIS3DH** nehmen; wenn er das Blubbern im Test klar zeigt, reicht er.
3. **Mechanik:** Sensor starr (verschraubt oder geklebt) auf der Unterseite der Deckplatte T1, möglichst **direkt unter dem Auflagering** des Glases, Z-Achse senkrecht. Weiche Füße unter dem Untersetzer entkoppeln vom Tisch: dann geht die Beschleunigung gegen F/M, und Tischklopfen wird gedämpft.
4. **Auswertung in der Firmware:** Z-Achse mit 400 Hz über den FIFO lesen, Bandpass 20 bis 150 Hz, Effektivwert über 0,25 bis 0,5 s, Schwelle adaptiv zum Ruhe-Rauschen (z. B. 3-faches Ruhe-Rauschen). Kurze, große Spitzen (Anstoßen, > 50 mg) getrennt erkennen und nicht als Zug werten.

## 6. Alternative, falls Vibration nicht reicht

Ein **MEMS-Mikrofon** (z. B. INMP441, I²S, ca. 2 bis 4 €) im Untersetzer hört das Blubbern direkt durch das Glas. Das Blubbern ist laut und hat ein typisches Spektrum. Nachteil: Musik und Gespräche müssen herausgefiltert werden. Als Reserve sinnvoll, nicht als erste Wahl.

## 7. Messtest

Stand 2026-11-21: LIS2DW12-Modul bestellt. Testsketch `test/b0_hardware/B0_Beschleunigung` (Anleitung und Messprotokoll in `test/b0_hardware/README.md`).

1. LIS2DW12 per I²C an GPIO 6/7, mit dünnem Folienband oder Kleber unter die Deckplatte direkt unter den Auflagering des Glases (kein Schaumklebeband).
2. Der Sketch prüft die Chip-Kennung (0x44), misst 10 s Ruhe und gibt danach den Effektivwert im Band 20 bis 150 Hz aus. Der LED-Ring zeigt den Pegel.
3. Abschnitte markieren: Ruhe, 5 Züge, Schlauch anfassen, Tisch antippen. Die Auswertung (`e`) rechnet den Faktor Zug/Ruhe aus; der SW-420 läuft zum Vergleich mit.
4. **Kriterium:** Zug mindestens 3-mal so stark wie Ruhe → reicht mit Reserve; 2- bis 3-mal → reicht knapp; darunter → Montage verbessern, sonst ADXL355 oder Mikrofon.

Diese Messung beantwortet in 15 Minuten, was die Rechnung nur auf Faktor 10 genau schätzen kann.
