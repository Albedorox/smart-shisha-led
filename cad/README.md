# CAD – Smart Shisha LED-Untersetzer

Erstellt mit SolidCraft (über MCP auf dem Rechner des Nutzers). Planungsgrundlage: `docs/planung/03_mechanical_design.md`, Workflow: `docs/planung/08_cad_mcp_plan.md`.

## Stand v4 (2026-10-27) – nur Testring

Befund des Nutzers am Testring v3: Das Glas lässt sich weiterhin ca. 6 mm hin und her schieben. Wunsch: Rand wieder so hoch wie bei v1 und 2–3 mm weiter außen. Der erste v4-Stand (Innen-Ø 101,1) war zu weit außen; auf Wunsch des Nutzers ist der Innendurchmesser jetzt 95 mm, die Breite wie bei v3.

| Parameter | v3 | v4 |
|---|---|---|
| `center_clear` | 3 mm | 1,55 mm (Außenkante Rand r 52, Ø 104; v1: Ø 105,1) |
| `center_rim_w` | 4,5 mm | 4,5 mm (Rand r 47,5–52, Innen-Ø 95; v3: 92,1, v1: 100,1) |
| `center_rim_h` | 1,2 mm | 2 mm |
| `center_rim_ch` / `center_rim_ch_w` | 0,7 / 2,5 mm (Schräge) | 1 / 1 mm (45°-Fase außen, wie v1) |

Nur `v4/stl/P_Testring_v4.stl` ist neu exportiert. T1 und T2 bleiben bis zur Rückmeldung v3. Das Modell `v4/SmartShisha_Untersetzer.solidcraft` ist mit der Zeitleiste hinter dem Testring gespeichert (Marke bei Feature 29). Vor dem Export von T1/T2 also „Zeitleiste ans Ende“ ausführen.

Hinweis: Der Rand von v1 (Außenkante Ø 105,1, 2 mm hoch) hat das Glas ca. 1,5 mm angehoben. v4 ist gleich hoch, die Außenkante liegt 0,55 mm weiter innen als bei v1. Das gemessene Spiel (v2: 8 mm, v3: 6 mm) passt zu einem Glasfuß-Innen-Ø von etwa 113 mm statt 107. Wenn v4 wieder kippelt, hilft eine Messung von Innen- und Außendurchmesser des Glasfußes.

## Stand v3 (2026-10-25) – Änderungen aus dem ersten Vollteil-Druck

Befund des Nutzers am gedruckten T1/T2 v2:

1. Das Glas rutscht ca. 8 mm zu jeder Seite, bis der Zentrierrand es aufhält. Der Rand sitzt zu weit innen und ist zu niedrig.
2. Die Sensorstifte (2 × 2 mm, 8,5 mm hoch) brechen ab.
3. Der SuperMini liegt lose im Rahmen. Stiftleisten sind vorgesehen, aber nicht nötig.
4. Die Nut in T1 ist zu knapp. Ihre flache Decke hängt beim Druck durch, das Filament drückt sich in den Spalt, und T1 lässt sich nicht aufsetzen.

Änderungen in `v3/` (T1, T2 und der Testring; T4 bleibt v1):

| Teil | Änderung | Parameter |
|---|---|---|
| T1 Zentrierrand | Er sitzt 2 mm weiter außen (Außen-Ø 101,1 statt 97,1) und ist 4,5 mm breit. Innen ist er 1,2 mm hoch, nach außen fällt er schräg auf 0,5 mm ab und folgt so der Glaswölbung mit ca. 0,3 mm Luft. Ein höherer Rand würde nach der Näherung wieder auf der Wölbung aufsitzen (v1: 1,5 mm Anheben). | `center_clear` 5 → 3, `center_rim_w` 2,5 → 4,5, `center_rim_h` 1 → 1,2, `center_rim_ch` 0,5 → 0,7 (senkrecht), neu `center_rim_ch_w` 2,5 (radial) |
| T1 Nut | 0,3 mm Spiel je Seite und oben (vorher 0,2). Die Decke ist ein 45°-Dach, damit sie ohne Stützmaterial nicht durchhängt. Am Nuteingang gibt es Fasen 0,3 mm gegen den Elefantenfuß. | `fit_slide` 0,2 → 0,3, neu `groove_ch` 0,3 |
| T2 Feder | Oben beidseitig 0,3 mm angefast (Einführhilfe). | neu `tongue_ch` 0,3 |
| T2 Sensorstifte | Der äußere Stift ist über eine Rippe (2 × 3,4 mm, volle Höhe) mit der Wand verbunden. Der innere Stift hat zur Mitte hin eine Stützrippe (2 × 4 × 5 mm). Die Rippen liegen auf der vom Sensor abgewandten Seite, also dort, wohin der Sensor beim Verdrehen drückt. | Features `T2_Sensor_Rippe_aussen`, `T2_Sensor_Rippe_innen` |
| T2 SuperMini | Die Rahmenwände reichen jetzt bis zur Platinenoberkante (z 5,1). An der Antennenseite federt eine Schnappzunge (0,8 × 8 mm, aus einer 1,4 mm tiefen Tasche im Boden, damit sie lang genug federt). Ihre Nase (0,4 mm über der Platine, 45°-Einlauf) hält die Platine. Einsetzen: zuerst die USB-Seite zur Wandöffnung, dann die Antennenseite herunterdrücken, bis es einrastet. | Features `T2_MCU_*_hoch`, `T2_MCU_Schnapper_*`, `Chamfer1` |

**Stiftleisten:** Mit nach unten gelöteten Stiftleisten ist der SuperMini 12,7 mm hoch. Im Gehäuse sind zwischen Boden und T1 nur 10 mm Platz. Die Leisten müssen daher ab, und die Kabel werden direkt an die Pads gelötet (wie bisher geplant). Alternativ müsste das Gehäuse etwa 3 mm höher werden.

**Glaswölbung:** Sie ist weiterhin als Kugelkappe genähert (7,5 mm tief, R ≈ 195 mm). Laut dieser Näherung berührt das Glas den neuen Rand nach etwa 1 mm Versatz; ab dann müsste es sich anheben, um weiterzurutschen. Spätestens nach 3 mm stößt der Glasfuß an die Außenkante des Rands. Dass das Glas bei v2 frei rutschte, deutet darauf hin, dass die echte Wölbung am Rand eher höher ist als die Näherung, was dem neuen Rand nicht schadet. Der Testring v3 (ca. 29 cm³) prüft das ohne komplette Deckplatte.

| Datei | Inhalt |
|---|---|
| `v3/SmartShisha_Untersetzer.solidcraft` | Quellmodell v3 |
| `v3/parameter.csv`, `v3/SmartShisha_Untersetzer.script.json` | Parameter und Zeitleiste v3 |
| `v3/stl/T1_Deckplatte_v3.stl` | Deckplatte v3 (Glasauflage nach oben drucken) |
| `v3/stl/T2_Unterschale_v3.stl` | Unterschale v3 (Boden auf dem Bett) |
| `v3/stl/P_Testring_v3.stl` | Zentrier-Testring v3 |
| `v3/ansicht_schnitt_rand_nut.png` | Schnitt Zentrierrand mit Glasnäherung sowie Nut/Feder |
| `v3/ansicht_esp32_schnapper.png`, `v3/ansicht_sensorstift.png` | Ansichten SuperMini-Halter und Sensorstift |

Prüfung (trimesh): T1, T2 und Testring geschlossen, Normalen konsistent. Keine Überschneidung T1 ↔ T2, Platine SuperMini ↔ T2 und SW-420 ↔ T2. Nut/Feder: 0,3 mm seitlich, 0,3 mm oben (1,1 mm bis zur Dachspitze).

## Stand v2 (2026-10-24) – Änderungen aus Testdruck 1 (Testring)

Befund des Nutzers am gedruckten Testring v1:

1. Das Glas kippelt. Der Zentrierrand hebt es an; liegt das Glas auf einer Seite auf, steht es auf der anderen ca. 1,5 mm hoch.
2. Der LED-Ring passt nur sehr knapp in die Einfassung.
3. Das runde Kabelloch (Ø 6) verdeckt die Lötpads teilweise und ist zu klein.

Änderungen in `v2/` (nur T1 und der daraus abgeleitete Testring sind betroffen; T2 und T4 bleiben v1):

| Parameter | v1 | v2 | Grund |
|---|---|---|---|
| `center_clear` | 1 mm | 5 mm | Zentrierrand 4 mm weiter innen (Außen-Ø 105,1 → 97,1) |
| `center_rim_h` / `center_rim_ch` | 2 / 1 mm | 1 / 0,5 mm | Rand niedriger, damit er die Glaswölbung nicht berührt |
| `ring_clear` | 0,3 mm | 0,8 mm | Einfassung Ø 86,6 → 87,6 (+1 mm) |
| `cable_hole_d`, `cable_hole_r` | Rundloch Ø 6 | entfallen | ersetzt durch Rechteck |
| `cable_slot_l` × `cable_slot_w`, `cable_slot_r` | – | 14 × 6,6 mm, Mitte r 39,7 | rechteckige Aussparung unter allen 4 Pads (DI, 5V, GND, DO; Padreihe ca. 8,6 mm) |
| `sens_rib_h` | 3 mm | entfallen | wurde nicht mehr verwendet |
| `fit_slide` | 0,3 mm | 0,2 mm | Coupon: Stift Ø10 passt am besten in Ø10,4 (Nut T1 für die Feder von T2) |

**Glas-Hüllkörper:** Die Wölbung ist jetzt als Kugelkappe genähert (7,5 mm tief über dem Fuß-Innenradius 53,55 mm, R ≈ 195 mm). Mit dieser Form ergibt der alte Rand genau die gemessenen ca. 1,5 mm Anheben. Prüfung v2 (Strahlentest gegen die exportierten Netze): Das Glas steht mittig ca. 0,5 mm über dem Rand und berührt ihn erst bei ca. 2 mm Versatz. Luft Glas ↔ LED-Ring-Hüllkörper mindestens 1,1 mm (v1-Angabe 3,8 mm beruhte auf der alten, zu steilen Näherung).

| Datei | Inhalt |
|---|---|
| `v2/SmartShisha_Untersetzer.solidcraft` | Quellmodell v2 |
| `v2/parameter.csv`, `v2/SmartShisha_Untersetzer.script.json` | Parameter und Zeitleiste v2 |
| `v2/stl/P_Testring_v2.stl` | Zentrier-Testring v2 (zum erneuten Prüfen, ca. 29 cm³) |
| `v2/stl/T1_Deckplatte_v2.stl` | Deckplatte v2 |

**Coupon-Ergebnisse (Nutzer 2026-10-24):** Stift Ø10 → Bohrung Ø10,4 (`fit_slide` 0,2), M3-Schraube → Ø3,4 (`screw_clear_d` bleibt). Einschmelzmuttern noch nicht getestet; angenommen wird jeweils die mittlere Bohrung, also die bisherigen Werte (`ins_m2_d` 3,2, `ins_m3_d` 4,0). `fit_slide` wirkt nur auf die Nut in T1; T2 ist geometrisch unverändert (gleiches Netz wie v1). Den Testring v2 druckt der Nutzer nicht; die Änderungen gelten als angenommen.

Netzprüfung (trimesh): T1 v2 und Testring v2 geschlossen, Normalen konsistent, ebene Unterseite (T1 ca. 11 470 mm²).

## Stand v1 (2026-10-24) – Vollteile exportiert

G3 vom Nutzer am 2026-10-24 freigegeben. Die Vollteile T1, T2 und T4 wurden auf Wunsch des Nutzers exportiert, bevor die Testdrucke ausgewertet waren. Deshalb gelten noch die Startwerte für die Passungen (Einschmelzmuttern M2 Ø3,2 / M3 Ø4,0, M3-Durchgang Ø3,4, Gleitpassung 0,3 mm). Weichen die Coupon-Ergebnisse ab, wird neu exportiert (v2).

| Datei | Inhalt |
|---|---|
| `v1/SmartShisha_Untersetzer.solidcraft` | Quellmodell (parametrisch) |
| `v1/parameter.csv` | alle Benutzerparameter mit Ausdruck und Quelle (gemessen / A = Annahme / S = Vorschlag) |
| `v1/SmartShisha_Untersetzer.script.json` | Zeitleiste als Skript (nachvollziehbar, ohne SolidCraft lesbar) |
| `v1/ansicht_schnitt.png` | Schnitt durch Glas, LED-Ring, Zentrierrand, Schraubdom |
| `v1/ansicht_iso.png` | Isometrische Ansicht mit Hüllkörpern |
| `v1/stl/P_Testring_v1.stl` | Zentrier-Testring (T-CAD-07) |
| `v1/stl/P_Coupon_v1.stl`, `v1/stl/P_Coupon_Stift_v1.stl` | Toleranzcoupon mit Stift (T-CAD-08) |
| `v1/stl/T1_Deckplatte_v1.stl` | Deckplatte (Glasauflage nach oben drucken) |
| `v1/stl/T2_Unterschale_v1.stl` | Unterschale (Boden auf dem Bett) |
| `v1/stl/T4_Distanzring_v1.stl` | Distanzring Sensor, Ø7 × 1,5 mm |
| `v1/ansicht_unterschale_innen.png` | Unterschale innen mit SuperMini, Sensor, USB-C-Breakout |

## Aufbau

- **T1 Deckplatte** (z 12–18, Ø125): Glasauflage oben, Zentrierrand Ø97,1 × 1 mm mit 0,5 mm Fase (v1: Ø105,1 × 2 mm), LED-Ring liegt in einer 1,6 mm tiefen Einfassung (Ø87,6; v1: Ø86,6). 4 × M2-Einschmelzmutter für den Ring, rechteckige Kabelaussparung 14 × 6,6 mm unter den Pads (v1: Ø6), 4 × M3-Einschmelzmutter (Lochkreis r 47) für die Verschraubung mit T2, 1 × M3 für den Sensor.
- **T2 Unterschale** (z 0–13,2): Boden 2 mm, Wand 2,4 mm, Feder 1 × 1,2 mm in Nut von T1 (Spiel 0,3 mm), 4 Schraubdome Ø8 mit Senkung von unten, 4 Mulden für Silikonfüße, 2 Ablauflöcher.
  - **Stromeingang (+X):** USB-C-Breakout 21,45 × 12,5 mm (rotes Modul mit 5,1 kΩ an CC, Lochabstand 16 mm laut Datenblatt) auf zwei Sockeln mit Kernloch Ø2,2 für M2,5-Schrauben (selbstschneidend, ca. 6 mm). Die Platine steckt mit der Kante in einer Tasche in der Wand, die Buchse sitzt in einer Öffnung 10 × 4,6 mm nahezu bündig mit der Außenfläche. Der Stecker braucht so keine große Öffnung für die Umspritzung. Angeschlossen werden nur VBUS und GND.
  - **Programmieranschluss (−X):** ESP32-C3 SuperMini in einem Rahmen auf einer Mittelrippe (1,5 mm Luft für Lötstellen), USB-C direkt an einer Wandöffnung 12,5 × 7,5 mm für den Stecker. Fixierung mit Heißkleber oder Schaumklebeband. Beim Programmieren Powerbank abziehen (R-15).
  - **Sensor:** Zwei Stifte 2 × 2 mm neben dem SW-420 verhindern, dass er sich um seine Schraube dreht.
- **T4 Distanzring**: zwischen Sensor und Deckplatte, M3-Schraube durch Sensor und Ring in die Einschmelzmutter.
- **Hüllkörper** (nicht druckbar): LED-Ring, Glasboden (Näherung), ESP32-C3 SuperMini, SW-420, USB-C-Breakout.

## Prüfungen

- Abstandsprüfung (`inspect.clearance`): keine Überlappungen. Glas ↔ LED-Ring 3,77 mm, SW-420 ↔ Boden T2 0,5 mm. Der Hüllkörper des Breakouts ist ein grober Quader und ragt oberhalb der Platine in die Wand; dort sitzt nur die Buchse, und die liegt in der Öffnung.
- Netzprüfung (trimesh): T1, T2, T4 geschlossen, Normalen konsistent; T1 und T2 liegen mit einer ebenen Unterseite auf dem Bett.
- Druckrichtung: T1 mit der Glasauflage nach oben (flache Unterseite auf dem Bett), T2 mit dem Boden auf dem Bett; beide ohne Stützmaterial vorgesehen (Druckeinstellungen: `docs/planung/03_mechanical_design.md` §13).

## Offen

- Einschmelzmuttern am Coupon prüfen (`ins_m2_d`, `ins_m3_d`); bisher angenommen
- Breakout nach Lieferung nachmessen: Lochabstand, Lochdurchmesser, Überstand und Höhe der Buchse
- Höhe LED-Ring + LEDs (M-03b) – derzeit 1,6 + 1,6 mm angenommen
- Glasboden ist nur als Näherung modelliert; Bestätigung über den Zentrier-Testring

## Testdrucke (T-CAD-07, T-CAD-08)

Beide Teile sind in SolidCraft aus dem Modell abgeleitet (Zeitleiste ab „Move1“) und folgen damit Parameteränderungen. Netzprüfung mit trimesh: geschlossen, Normalen konsistent, Maße wie erwartet.

**Zentrier-Testring** (`P_Testring_v1.stl`, 125 × 125 × 5,6 mm, ca. 30 cm³): Oberer Teil der Deckplatte mit Glasauflage, Zentrierrand, Ring-Einfassung, M2-Löchern und Kabelloch. Innen ab Ø 59,8 offen, unten auf 2 mm unter der Einfassung gekürzt. Druck mit der flachen Seite auf dem Bett, ohne Stützen.

Prüfen:
- Glas aufsetzen: steht es ohne Klemmen und ohne Kippeln? Spiel zum Zentrierrand ca. 1 mm je Seite?
- LED-Ring einlegen: passt er in die Einfassung (0,3 mm Spiel), schließt er bündig ab, liegen die M2-Löcher über den Ringlöchern?
- Glas auf den eingelegten Ring stellen: berührt das Glas die LEDs? (Soll: ca. 3,8 mm Luft)

**Toleranzcoupon** (`P_Coupon_v1.stl`, 90 × 24 × 5 mm, und `P_Coupon_Stift_v1.stl`, Ø 10 × 8 mm). Die abgeschnittene Ecke markiert links vorne.

| Reihe | Bohrungen von links nach rechts | Prüfen |
|---|---|---|
| vorne, 1–3 | Ø 3,0 / 3,2 / 3,4, 4 mm tief | M2-Einschmelzmutter: welche Bohrung hält sie am besten? |
| vorne, 4–6 | Ø 3,8 / 4,0 / 4,2, 4,5 mm tief | M3-Einschmelzmutter |
| vorne, 7–9 | Ø 3,2 / 3,4 / 3,6, durchgehend | M3-Schraube: welche geht leicht durch? |
| hinten, 1–3 | Ø 10,4 / 10,6 / 10,8, durchgehend | Stift Ø 10: welche Bohrung ist eine gute Gleitpassung (0,2 / 0,3 / 0,4 mm Spiel je Seite)? |

Die Ergebnisse ergeben die Werte für `ins_m2_d`, `ins_m3_d`, `screw_clear_d` und `fit_slide`.
