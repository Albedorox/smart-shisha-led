# CAD – Smart Shisha LED-Untersetzer

Erstellt mit SolidCraft (über MCP auf dem Rechner des Nutzers). Planungsgrundlage: `docs/planung/03_mechanical_design.md`, Workflow: `docs/planung/08_cad_mcp_plan.md`.

## Stand v1 (2026-10-24) – G3 freigegeben, Testdrucke

G3 vom Nutzer am 2026-10-24 freigegeben. Exportiert sind nur die Testteile (`v1/stl/`). Die Vollteile T1/T2 folgen nach bestandenen Testdrucken (G4) und den offenen Punkten unten.

| Datei | Inhalt |
|---|---|
| `v1/SmartShisha_Untersetzer.solidcraft` | Quellmodell (parametrisch) |
| `v1/parameter.csv` | alle Benutzerparameter mit Ausdruck und Quelle (gemessen / A = Annahme / S = Vorschlag) |
| `v1/SmartShisha_Untersetzer.script.json` | Zeitleiste als Skript (nachvollziehbar, ohne SolidCraft lesbar) |
| `v1/ansicht_schnitt.png` | Schnitt durch Glas, LED-Ring, Zentrierrand, Schraubdom |
| `v1/ansicht_iso.png` | Isometrische Ansicht mit Hüllkörpern |
| `v1/stl/P_Testring_v1.stl` | Zentrier-Testring (T-CAD-07) |
| `v1/stl/P_Coupon_v1.stl`, `v1/stl/P_Coupon_Stift_v1.stl` | Toleranzcoupon mit Stift (T-CAD-08) |

## Aufbau

- **T1 Deckplatte** (z 12–18, Ø125): Glasauflage oben, Zentrierrand Ø105,1 × 2 mm mit 1 mm Einführfase, LED-Ring liegt in einer 1,6 mm tiefen Einfassung (Ø86,6). 4 × M2-Einschmelzmutter für den Ring, Kabeldurchführung Ø6 unter den Pads, 4 × M3-Einschmelzmutter (Lochkreis r 47) für die Verschraubung mit T2, 1 × M3 für den Sensor.
- **T2 Unterschale** (z 0–13,2): Boden 2 mm, Wand 2,4 mm, Feder 1 × 1,2 mm in Nut von T1 (Spiel 0,3 mm), 4 Schraubdome Ø8 mit Senkung von unten, 4 Mulden für Silikonfüße, 2 Ablauflöcher, USB-C-Öffnung 9,5 × 3,8 in der Seitenwand.
- **Hüllkörper** (nicht druckbar): LED-Ring, Glasboden (Näherung), ESP32-C3 SuperMini, SW-420.

## Prüfungen

- Abstandsprüfung (`inspect.clearance`): keine Überlappungen. Glas ↔ LED-Ring 3,77 mm, SW-420 ↔ Boden T2 0,5 mm.
- Druckrichtung: T1 mit der Glasauflage nach oben (flache Unterseite auf dem Bett), T2 mit dem Boden auf dem Bett; beide ohne Stützmaterial vorgesehen (Druckeinstellungen: `docs/planung/03_mechanical_design.md` §13).

## Offen vor den Vollteilen

- Halterung SuperMini (Führungsschienen) und USB-C-Buchse (M-07b, Buchse noch nicht vermessen)
- Abstandshalter Sensor (T4)
- Höhe LED-Ring + LEDs (M-03b) – derzeit 1,6 + 1,6 mm angenommen
- Glasboden ist nur als Näherung modelliert; Bestätigung über den Zentrier-Testring
- Nächster Schritt nach G3: Toleranzcoupon und Zentrier-Testring als STL

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
