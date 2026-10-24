# CAD – Smart Shisha LED-Untersetzer

Erstellt mit SolidCraft (über MCP auf dem Rechner des Nutzers). Planungsgrundlage: `docs/planung/03_mechanical_design.md`, Workflow: `docs/planung/08_cad_mcp_plan.md`.

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

- **T1 Deckplatte** (z 12–18, Ø125): Glasauflage oben, Zentrierrand Ø105,1 × 2 mm mit 1 mm Einführfase, LED-Ring liegt in einer 1,6 mm tiefen Einfassung (Ø86,6). 4 × M2-Einschmelzmutter für den Ring, Kabeldurchführung Ø6 unter den Pads, 4 × M3-Einschmelzmutter (Lochkreis r 47) für die Verschraubung mit T2, 1 × M3 für den Sensor.
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

- Coupon-Ergebnisse übernehmen (`ins_m2_d`, `ins_m3_d`, `screw_clear_d`, `fit_slide`), ggf. v2 exportieren
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
