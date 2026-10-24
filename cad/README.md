# CAD – Smart Shisha LED-Untersetzer

Erstellt mit SolidCraft (über MCP auf dem Rechner des Nutzers). Planungsgrundlage: `docs/planung/03_mechanical_design.md`, Workflow: `docs/planung/08_cad_mcp_plan.md`.

## Stand v1 (Entwurf, 2026-10-24) – wartet auf Freigabe G3

Noch **keine** Druckdateien (STL/3MF/STEP). Export erst nach Prüfung und Freigabe G3.

| Datei | Inhalt |
|---|---|
| `v1/SmartShisha_Untersetzer.solidcraft` | Quellmodell (parametrisch) |
| `v1/parameter.csv` | alle Benutzerparameter mit Ausdruck und Quelle (gemessen / A = Annahme / S = Vorschlag) |
| `v1/SmartShisha_Untersetzer.script.json` | Zeitleiste als Skript (nachvollziehbar, ohne SolidCraft lesbar) |
| `v1/ansicht_schnitt.png` | Schnitt durch Glas, LED-Ring, Zentrierrand, Schraubdom |
| `v1/ansicht_iso.png` | Isometrische Ansicht mit Hüllkörpern |

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
