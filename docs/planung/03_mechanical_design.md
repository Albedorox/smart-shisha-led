# 03 – Mechanisches Konzept, parametrische Maße, Toleranzen und Montage

Stand: 2026-10-24 (Rev. 2, eingearbeitet: Messwerte des Nutzers zu Glas, LED-Ring, SuperMini, SW-420, Drucker) · Phase A · Es wurde **kein CAD-Modell erzeugt**.

Änderung gegenüber Rev. 1: Der Glasboden steht nicht auf einer Vollfläche von Ø 150 mm, sondern auf einem schmalen Auflagering von Ø 109,1 mm und ist innen konkav. Der vorhandene LED-Ring (24 LEDs, Ø 86 mm) passt in diese Wölbung. Das Konzept „Fenster in einer Glastasche“ entfällt; der Ring sitzt **oben auf dem Untersetzer** in einer flachen Einfassung, das Glas steht darüber.

## 1. Glas: gemessene Werte und Begriffe (Lastenheft §4.1)

| Begriff | Symbol | Wert | Status |
|---|---|---|---|
| Durchmesser am Auflagepunkt (Mitte der Auflage) | `glass_contact_d` | **109,1 mm** (an 3 Stellen gleich) | **gemessen** (Nutzer, 2026-10-24) |
| Breite der Auflage | `glass_contact_w` | ca. 2 mm | gemessen (ca.) |
| Innenkante der Auflage | `glass_contact_id` | `glass_contact_d − glass_contact_w` ≈ 107,1 mm | abgeleitet |
| Wölbung innen (konkav) | `glass_dome_h` | 7,5 mm in der Mitte; nahe der Auflage steil, nach innen abflachend | gemessen |
| Außendurchmesser der Auflage | `glass_contact_od` | ca. 113 mm | gemessen (Nutzer) |
| Flanke außen | `glass_flank_angle` | 36° zur Glasachse (= 54° zur Tischfläche) | gemessen |
| Maximaler Außendurchmesser des Glases | `glass_max_d` | 150 mm in ca. 30 mm Höhe über der Auflage | gemessen |
| Freiraum über dem Ring | – | Glas kippelt nicht auf dem Ring und lässt sich ca. 13 mm in alle Richtungen verschieben, ohne ihn zu berühren | gemessen (Nutzer) |
| Glasboden | – | klar, gut lichtdurchlässig | Beobachtung (Fotos) |
| Gewicht leer | – | ca. 900 g; Shisha ca. 50 cm hoch, leicht gebaut | gemessen (ca.) |

Der Wert 150 mm aus dem Lastenheft wird durch die Messung ersetzt. Er bleibt als Parameter `glass_max_d` für die Optik erhalten, bis M-09 vorliegt.

## 2. Lichtführung (D-01, entschieden 2026-10-24: von unten durch das Glas)

Konkrete Umsetzung nach den Messwerten:

```
              Glas (Flanke ~36°)                       Glas
               ╲                                       ╱
                ╲   konkaver Boden, bis 7,5 mm hoch   ╱
                 ╲________________________________ ╱
   Auflage Ø109,1 ▼  ┌▀▀▀▀▀▀ LED-Ring Ø86 ▀▀▀▀▀▀┐   ▼
 ════════════════╤═▲═╧═════════════════════════╧═▲═╤════════════════  Oberseite Deckplatte
                    └ Zentrierrand Ø105 (2 mm hoch) ┘
```

- Der LED-Ring liegt mit den LEDs nach oben in einer flachen Einfassung (Vertiefung = Platinendicke) und ist mit 4 × M2 in Einschmelzmuttern verschraubt.
- Das Glas steht mit seinem Auflagering auf der ebenen Oberseite außerhalb des Rings. Der LED-Ring trägt kein Gewicht.
- **Zentrierung:** Ein umlaufender Zentrierrand (Ø 105 mm, 2 mm hoch, Fase oben) liegt innerhalb der Auflage und greift in die Wölbung. Damit ist das Glas seitlich geführt, ohne an der Außenflanke zu klemmen; das radiale Spiel beträgt ca. 1 mm.
- Optional: Eine dünne transparente Abdeckscheibe (0,6–0,8 mm, klares PETG) über dem Ring als Schutz vor Schmutz. Vorschlag ohne Abdeckung, weil der Nutzer maximale Helligkeit möchte und kein Wasser auf den Untersetzer gelangt (D-02).

Voraussetzung (M-09): Die Wölbung muss über dem Ring (r ≤ 43 mm) mindestens LED-Höhe über Oberseite + 1 mm Luft und über dem Zentrierrand (r = 52,5 mm) mindestens 2 mm + 0,5 mm Luft frei lassen. Kann der Nutzer prüfen, indem er den Ring auf eine ebene Fläche legt, das Glas darüberstellt und auf Kippeln achtet; Testdruck T-CAD-07 bestätigt es.

## 3. Bauteile: gemessene Maße

| Bauteil | Maß | Status |
|---|---|---|
| **LED-Ring** (24 × WS2812 5050 mit integriertem Treiber) | Außen-Ø 86 mm, Innen-Ø 71,8 mm (Ringbreite 7,1 mm) | gemessen |
| | 4 Befestigungslöcher für M2, Lochmitten als Rechteck 64,65 × 49,6 mm (Lochkreis-Ø rechnerisch 81,4 mm) | gemessen |
| | Pads DI, 5V, GND, DO auf der Rückseite, nebeneinander an einer Stelle des Rings (Fotos: „oben“, zwischen den beiden oberen Löchern) | gemessen (Foto) |
| | Platinendicke und LED-Bauhöhe | **Annahme** 1,6 mm + 1,6 mm → M-03b |
| **ESP32-C3 SuperMini** | 22,7 × 18,1 mm; 24,0 mm mit USB-C; Höhe Platine + USB 4,4 mm; Unterseite unbestückt; 12,7 mm mit nach unten gelöteten Stiftleisten | gemessen |
| | Vorschlag: Kabel direkt anlöten, **keine** Stiftleisten → Bauhöhe 4,4 mm | Vorschlag (Nutzer einverstanden) |
| **SW-420-Modul** | 31,2 × 14,2 mm; 1 Loch M3, Mitte 6,8 mm von der vorderen Platinenkante (Stiftleisten-Seite) und mittig in der Breite | gemessen; „mittig in der Breite“ aus Foto abgeleitet |
| | Bauhöhe: Platine ca. 1,6 mm, Poti ca. 5 mm, Federkontakt liegend ca. Ø 6 mm; Winkel-Stiftleiste ragt seitlich ca. 8 mm über | **Annahme** aus Foto → M-04b |
| **Drucker** | Anycubic Kobra 2 Neo, Bauraum laut Hersteller 220 × 220 × 250 mm, Düse 0,4 mm, Filament 1,75 mm PLA/PETG | gemessen/angegeben |

## 4. Parametersatz (zentrale Parameterdatei)

Alle Längen in mm. Status: **G** = gemessen/gegeben, **A** = Annahme, **S** = Startwert (durch Testdruck zu bestätigen), **M** = Messaufgabe offen, **D** = abgeleitet.

| Gruppe | Parameter | Wert | Status | Bemerkung |
|---|---|---|---|---|
| Glas | `glass_contact_d` | 109,1 | G | |
| | `glass_contact_w` | 2,0 | G | |
| | `glass_max_d` | 150 | G | nur Hüllkörper |
| Zentrierung | `center_clear` | 1,0 | S | radiales Spiel zur Auflage-Innenkante |
| | `center_rim_od` | `glass_contact_d − glass_contact_w − 2·center_clear` ≈ 105,1 | D | |
| | `center_rim_w` | 2,5 | S | Wandstärke des Zentrierrands |
| | `center_rim_h` | 2,0 | S | M-09 |
| | `center_rim_chamfer` | 1,0 | S | Einführfase |
| Außenform | `outer_d` | 125 | G | vom Nutzer bestätigt (D-31); Glasflanke liegt bei r = 62,5 mm ca. 8 mm über der Oberseite |
| | `edge_chamfer` | 1,5 | S | |
| | `total_h` | `foot_h + base_t + cavity_h + top_t` ≈ 19 (+ Zentrierrand 2) | D | |
| Deckplatte T1 | `top_t` | 5,0 | S | trägt Glas, nimmt M2-Einsätze auf |
| | `ring_recess` | 1,6 | S | = Platinendicke Ring (M-03b) |
| | `ring_recess_clear` | 0,3 | S | radial je Seite |
| | `cable_hole_d` | 6,0 | S | unter den Pads |
| Ring | `led_count` | 24 | G | |
| | `ring_od` / `ring_id` | 86,0 / 71,8 | G | |
| | `ring_hole_dx` / `ring_hole_dy` | 64,65 / 49,6 | G | Rechteck der Lochmitten |
| | `ring_pad_angle` | 90° (oben, zwischen den oberen Löchern) | G (Foto) | Lage des Kabellochs |
| | `ring_pcb_t` / `led_h` | 1,6 / 1,6 | A | M-03b |
| Unterschale T2 | `base_t` | 2,0 | S | |
| | `wall_t` | 2,4 | S | 6 Perimeter bei 0,4-mm-Düse |
| | `cavity_h` | 10,0 | S | Sensor ca. 8 mm hoch + Luft |
| | `boss_count` | 4 | S | Verschraubung T1/T2 |
| | `drain_d` | 3,0 | S | 2 Ablauföffnungen |
| Elektronik | `mcu_l` / `mcu_w` / `mcu_h` | 22,7 / 18,1 / 4,4 | G | |
| | `mcu_l_usb` | 24,0 | G | |
| | `mcu_clear` | 0,3 | S | je Seite |
| Sensor | `sen_l` / `sen_w` / `sen_h` | 31,2 / 14,2 / 8,0 | G / G / A | |
| | `sen_hole_d` / `sen_hole_x` | 3,2 (M3) / 6,8 von vorderer Kante | G | |
| Versorgung | `usb_in` | USB-C-Einbaubuchse auf Breakout mit 5,1-kΩ-CC-Widerständen | S | D-06 |
| | `usb_in_l` / `usb_in_w` / `usb_in_hole_dx` | abhängig vom gekauften Breakout | M | M-07b |
| Befestigung | Ring | 4 × M2 × 6 in M2-Einschmelzmuttern (Bohrung typ. 3,2 mm, Länge 3 mm) | S | Werte nach Datenblatt |
| | T1/T2 | 4 × M3 × 12 von unten in M3-Einschmelzmuttern in T1 | S | |
| | Sensor | 1 × M3 × 8 in Einschmelzmutter, über Distanzhülse an T1 | S | |
| Standfläche | `foot_type` | 4–6 Silikonpuffer Ø 10 × 2 oder Moosgummiring | S | D-15 |
| | `foot_h` | 2,0 | S | |
| Fertigung | `nozzle_d` / `layer_h` | 0,4 / 0,2 | G / S | |
| | `fit_slide` / `fit_press` | 0,30 / 0,10 | S | Toleranzcoupon T-CAD-08 |
| | `bed` | 220 × 220 | G | `outer_d` passt |

Regeln: Abgeleitete Werte nur über Formeln. Jede Messung ersetzt den Annahmewert mit Datum im Kommentar des Parameters (SolidCraft unterstützt Parameterkommentare).

## 5. Aufbau und Teile

Höhenaufbau (Startwerte):

```
 z (mm)
 21 ┤         ┌┐ Zentrierrand Ø105                         ┌┐
 19 ┤═════════╧╧══[  LED-Ring in 1,6-mm-Einfassung  ]══════╧╧══════  Oberseite T1 (Glas steht hier auf Ø109,1)
    │  Deckplatte T1, 5 mm, massiv gedruckt (Infill)                 Einschmelzmuttern von unten
 14 ┤────────────────────────────────────────────────────────────
    │  Hohlraum 10 mm: SuperMini, Pegelwandler, Sensor (an T1), Kabel, USB-C-Eingang in der Wand
  4 ┤────────────────────────────────────────────────────────────
  2 ┤  Boden Unterschale T2, 2 mm
  0 ┤  Füße / Rutschring
```

| Nr. | Baugruppe (Lastenheft §4.3) | Teil | Funktion und Konstruktion | Druckorientierung |
|---|---|---|---|---|
| 1 | Grundkörper | T1 Deckplatte + T2 Unterschale | T1: Glasauflage, Zentrierrand, Ring-Einfassung, Kabelloch, Einsatzbohrungen. T2: Boden, Außenwand, Schraubdome, Elektronikfach, Öffnung für USB-C-Eingang | T1 Oberseite nach oben, flache Unterseite auf dem Bett (Einsatzbohrungen von unten = keine Stützen). T2 Boden auf dem Bett |
| 2 | LED-Träger | in T1 | Einfassung zentriert den Ring, 4 M2-Einsätze halten ihn; Kabelloch unter den Pads | – |
| 3 | Elektronikaufnahme | in T2 | Tasche mit `mcu_clear`, zwei Rastnasen, Niederhalter; SuperMini liegt flach (ohne Stiftleisten); Antenne zur Außenwand, nicht unter Metall | Teil von T2 |
| 4 | Sensorhalter | T4 | kleine Distanzhülse/Winkel, verschraubt den SW-420 mit M3 starr an der Unterseite von T1, nahe dem Auflagering (r ≈ 45–50 mm), damit Vibrationen des Glases direkt übertragen werden; Poti nach Abnehmen von T2 erreichbar | flach |
| 5 | Kabelwege | in T2 | Kanäle mit Kabelbinder-Ösen, Biegeradius ≥ 5 mm, Platz für die Lötverbindungen | – |
| 6 | Servicezugang | Trennung T1/T2 | 4 Schrauben von unten lösen → T1 mit Ring und Sensor abheben; Firmware-Update über USB-C des SuperMini bei geöffnetem Gehäuse | – |
| 7 | Befestigungselemente | Einsätze, Schrauben, T6 | M2-Einsätze (Ring), M3-Einsätze (T1/T2, Sensor); T6 = Halter für die USB-C-Einbaubuchse | – |
| 8 | Standfläche | Füße / Ring | Silikonpuffer bzw. Moosgummiring unter der Glasauflage | – |
| opt. | Abdeckung Ring | T5 | dünne klare Scheibe über dem Ring (D-02) | flach |

Begründung zwei Hauptteile: Beide Teile drucken ohne Stützstrukturen, die Elektronik bleibt zugänglich, es entsteht kein geschlossener Hohlraum.

## 6. Feuchtigkeit

Nach Aussage des Nutzers gelangt im Betrieb kein Wasser auf den Untersetzer. Daher keine Dichtungskonstruktion, aber einfache Maßnahmen ohne Mehraufwand:
- Kabelloch unter dem Ring nach der Montage mit Heißkleber/Silikon verschließen.
- Fuge T1/T2 mit kleiner Überlappung (Nut-Feder 1,2 mm), damit Kondenswasser außen abläuft.
- Zwei Ø 3-mm-Öffnungen im Boden von T2 außerhalb des Elektronikfachs (kein geschlossener Hohlraum).

## 7. Material und Wärme

| Material | Bewertung |
|---|---|
| **PETG (Vorschlag)** | erweicht ab ca. 75–80 °C, wasserbeständig, zäh; auf dem Kobra 2 Neo gut druckbar |
| PLA | möglich, da die Last klein ist (Glas befüllt ca. 2–3 kg auf 4 mm² × Umfang); erweicht aber ab ca. 55–60 °C (z. B. im Auto im Sommer) |

Kein Material wird als flammhemmend angenommen. Die LEDs liegen direkt unter dem Glas; bei Stromgrenze 500 mA (D-05) entstehen max. ca. 2,5 W, verteilt über 24 LEDs → Temperaturmessung T-HW-07.

## 8. Toleranzen (FDM, Startwerte – Coupon T-CAD-08)

| Passung | Spiel je Seite |
|---|---|
| Gleitpassung (Nut/Feder T1/T2) | 0,30 mm |
| Ring-Einfassung | 0,30 mm |
| Platinentaschen (SuperMini, Sensor) | 0,30 mm |
| Rastnasen | Hinterschnitt 0,6 mm, Federlänge ≥ 8 mm |
| Durchgangsbohrung M2 / M3 | Ø 2,4 / Ø 3,4 mm |
| Einschmelzmuttern M2 / M3 | Ø nach Datenblatt (typ. 3,2 / 4,0–4,2 mm) |
| Zentrierrand zum Glas | 1,0 mm radial (`center_clear`) |

## 9. Stabilität

- Last: Shisha befüllt geschätzt 2–3 kg (Glas leer 900 g), Höhe ca. 50 cm.
- Der Untersetzer erhöht die Shisha um ca. 19 mm und ist mit Ø 125 mm breiter als die Glasauflage (109,1 mm) → die Standfläche wird nicht kleiner.
- Füße liegen unter dem Auflagering → kurzer Lastpfad. Lasttest T-CAD-12 mit 5 kg (statt 10 kg).

## 10. Montageablauf (Entwurf, Detail in Phase B Dokument 12)

1. Einschmelzmuttern in T1 setzen: 4 × M2 von oben in die Ring-Einfassung, 4 × M3 und 1 × M3 (Sensor) von unten.
2. Kabel (3-adrig: DI, 5V, GND) an die Ring-Pads löten, durch das Kabelloch führen, Ring mit 4 × M2 verschrauben, Loch abdichten.
3. Sensor mit T4 an T1 schrauben, Poti-Grundeinstellung (Dokument 14).
4. USB-C-Einbaubuchse in T2 befestigen (T6).
5. SuperMini, Pegelwandler (+ Kondensator) einsetzen und verdrahten (Dokument 13).
6. Funktionstest vor dem Verschließen (Dokument 15).
7. T1 aufsetzen, 4 × M3 von unten.
8. Füße/Ring aufkleben.

## 11. Messaufgaben Mechanik

| ID | Aufgabe | Status |
|---|---|---|
| M-02 | SuperMini-Maße | **erledigt** 2026-10-24 |
| M-03 | LED-Ring: Ø, Löcher, Padlage | **erledigt** 2026-10-24 |
| M-03b | LED-Ring: Platinendicke und Gesamthöhe mit LED | offen |
| M-04 | SW-420: Maße, Lochlage | **erledigt** 2026-10-24 |
| M-04b | SW-420: Gesamthöhe (Poti), Bestätigung „Loch mittig in der Breite“ | offen |
| M-06 | Glas: Auflage, Wölbung, Gewicht, Durchsichtigkeit | **erledigt** 2026-10-24 |
| M-07b | Maße der gewählten USB-C-Einbaubuchse | offen (nach Kauf) |
| M-08 | Drucker und Filament | **erledigt** 2026-10-24 |
| M-09 | Glas: max. Außendurchmesser, Winkel, Kippel-Test | **erledigt** 2026-10-24 |

## 12. Prüfungen vor CAD-Export (Freigabepunkt G3, siehe 11)

- Alle offenen M-Aufgaben erledigt oder bewusst als Annahme freigegeben.
- Kollisionsprüfung (SolidCraft `inspect.interference` / `inspect.clearance`): Glas-Hüllkörper ↔ LED-Ring ↔ Zentrierrand; SuperMini, Sensor, USB-C-Buchse und Kabel-Hüllkörper im Hohlraum.
- Mindestwandstärke ≥ 1,6 mm; Überhänge ≤ 45° ohne Stütze; Brücken ≤ 20 mm.
- Kein geschlossener Hohlraum.

## 13. Druckempfehlung (PrusaSlicer, Anycubic Kobra 2 Neo, 0,4-mm-Düse)

Ausgangswerte für die eigenen Profile des Nutzers. Alle Teile sind so konstruiert, dass sie **ohne Stützstrukturen** drucken.

| Einstellung | PETG (empfohlen) | PLA (Alternative) | Begründung |
|---|---|---|---|
| Schichthöhe | 0,20 mm (T1 optional 0,16 mm für glattere Fase am Zentrierrand) | 0,20 mm | gutes Verhältnis Zeit/Qualität |
| Erste Schicht | 0,24 mm | 0,24 mm | Haftung |
| Extrusionsbreite | 0,45 mm | 0,45 mm | |
| Perimeter | 4 (≈ 1,7 mm Wand) | 4 | Stabilität um Einschmelzmuttern und Schraubdome |
| Obere Schichten | 5 (1,0 mm) | 5 | geschlossene, ebene Glasauflage |
| Untere Schichten | 4 (0,8 mm) | 4 | |
| Füllung | 25 % Gyroid | 20 % Gyroid | T1 trägt das Glas; Gyroid ist richtungsunabhängig |
| Stützstrukturen | **aus** | aus | Teile sind stützfrei konstruiert |
| Brim | 5 mm nur bei T2, falls sich Ecken lösen | meist nicht nötig | PETG verzieht wenig |
| Düse / Bett | 235–240 °C / 75–80 °C | 205–210 °C / 60 °C | Herstellerangaben des Filaments haben Vorrang |
| Lüfter | 30–50 % (Brücken 100 %) | 100 % | PETG haftet bei weniger Kühlung besser zwischen den Schichten |
| Geschwindigkeit | Außenwand 40 mm/s, Rest 60–80 mm/s | Außenwand 50, Rest 100 mm/s | Maßhaltigkeit der Passungen |
| Naht | „Hinten“ bzw. „Ausgerichtet“ | gleich | sichtbare Naht auf der Rückseite |
| Bügeln (Ironing) | optional nur oberste Fläche von T1 | optional | glattere Auflage, mehr Druckzeit |
| Druckbett | PEI: PETG mit Trennmittel (Klebestift) | direkt | PETG haftet sonst zu stark auf PEI |

Druckorientierung: T1 mit der Glasauflage nach oben, T2 mit dem Boden auf dem Bett, T4/T6 flach. Einschmelzmuttern nach dem Druck mit dem Lötkolben (ca. 220–230 °C bei PETG) senkrecht einsetzen.

Empfohlene Reihenfolge: Toleranzcoupon → Zentrier-Testring → T1 → T2.
