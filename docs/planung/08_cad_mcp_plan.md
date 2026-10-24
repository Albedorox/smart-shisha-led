# 08 – SolidCraft-MCP: Verifikation und CAD-Workflow

Stand: 2026-10-24 (Rev. 2) · Phase A

## 1. Ergebnis der Werkzeugprüfung

| Datum | Ergebnis |
|---|---|
| 2026-10-23 | In der Cloud-Arbeitsumgebung war kein SolidCraft-Server sichtbar (keine Tools, keine Connectoren, kein verbundener Rechner). |
| **2026-10-24** | Nach Verbindung mit dem Rechner des Nutzers ist SolidCraft **über MCP erreichbar**. Die Prüfung erfolgte **nur lesend**: Es wurde kein Design angelegt, geöffnet, geändert, gespeichert oder exportiert. |

Lesende Aufrufe am 2026-10-24: `running` (keine laufenden Aufträge), `list_commands` (vollständiger Katalog, 404 Befehle, vollständig ausgewertet), `describe_command engine.describe`, `execute file.slicers` (Abfrage installierter Slicer).

Beobachtungen zur Umgebung: Rechner mit Windows (Programmpfade `C:\Program Files\…`). Gefundener Slicer: **UltiMaker Cura 4.13.1**. Ob darin ein Profil für den Anycubic Kobra 2 Neo eingerichtet ist oder ob der Nutzer einen anderen Slicer verwendet, ist offen (Frage in 10).

## 2. Status der Prüfpunkte aus Lastenheft §4.6

„Dokumentiert“ = im Befehlskatalog vorhanden und beschrieben. „Funktionstest“ = in einem Wegwerf-Design ausprobiert (erfolgt in B5, Protokoll §3).

| Prüfpunkt | Befund (Katalog) | Funktionstest |
|---|---|---|
| CAD-Erstellung | Skizzen (106 Befehle: Linien, Kreise, Bögen, Rechtecke, Polygone, Langlöcher, Bemaßungen, Zwangsbedingungen), Volumen (`solid.extrude`, `revolve`, `sweep`, `loft`, `shell`, `fillet`, `chamfer`, `hole`, `thread`, `rib`, `pattern.circular`, `mirror`, `combine`, `split_body`) | offen |
| Parametrische Geometrie | Benutzerparameter mit Ausdrücken und Einheiten (`parameters.add/change/export/import`, `set_parameter`), Bemaßungen mit Parameterausdrücken, Zeitleiste mit Neuberechnung | offen |
| Zentrale Parameter ändern | `set_parameter` berechnet die Zeitleiste neu und meldet fehlschlagende Features | offen |
| Kunststoff-Konstruktionshilfen | `plastic.boss` (Schraubdome mit Rippen), `plastic.snap_fit` (Rastnasen), `plastic.lip` (Nut/Feder an Schalen) | offen |
| Normteile | `parts.fastener` / `parts.insert` (Schrauben, Muttern; ob Einschmelzmuttern enthalten sind, ist offen) | offen |
| Baugruppen | Komponenten, Gelenke, Explosionsdarstellung (`explode.create`), Stückliste (`assembly.bom`) | offen |
| Geometrie- und Kollisionsprüfung | `inspect.interference` (Überlappungsvolumen je Körperpaar), `inspect.clearance` (Mindestabstände), `inspect.section` (Schnitte), `inspect.draft` (Formschrägen/Überhang-Analyse nach Zugrichtung), `measure`, `body_topology` | offen |
| Exportformate | STEP, 3MF, 3MF mit Flächenkennung, STL (binär/ASCII), OBJ, URDF; Parameter als CSV/JSON; Bauteilskript (`timeline.export_script`, Komponenten noch nicht unterstützt) | offen |
| STL/3MF | `export` bzw. `file.save_mesh`, optional direkt im Slicer öffnen | offen |
| Druckbarkeitskontrolle | keine eigene Druckbarkeitsanalyse im Katalog; Hilfen: `inspect.draft` (Überhänge), Schnitte, Messungen; Übergabe an Cura | offen |
| Ansichten | `screenshot` (Fenster oder Modell-Rendering, Standardansichten), Schnitte | offen |
| Einschränkungen | Pfade sind Pfade auf dem Windows-Rechner des Nutzers; `new_design`/`open` verwerfen ungespeicherte Änderungen im gerade offenen Design; lange Aufrufe können per `running` geprüft werden; Absturzberichte über `crash_reports` | dokumentiert |

Folgerung: Der Katalog deckt alle in 03 benötigten Operationen ab (Drehkörper für Zentrierrand und Platten, Taschen, Bohrungen für Einschmelzmuttern, Schraubdome, Rastnasen, Nut/Feder, Interferenzprüfung, Export STEP + 3MF/STL). Der Ersatzweg CadQuery (D-16) wird nicht mehr benötigt, bleibt aber als Rückfallebene dokumentiert.

## 2a. Ergebnisse der Funktionstests (2026-10-24, nach Freigabe Phase B)

Durchgeführt in einem separaten Wegwerf-Design „V_Funktionstest“ (beim Start war in SolidCraft nur ein leeres, ungespeichertes Design offen).

| Nr. | Ergebnis |
|---|---|
| V-02 | ✓ Parameter `d`, Kreis per Bemaßung an `d` gebunden, Extrusion; Volumen plausibel |
| V-03 | ✓ `set_parameter d = 30` → Neuberechnung ohne Fehler, Bounding Box ± 15 mm |
| V-04 | ✓ Bohrung (`solid.hole` braucht `diameter`), Drehkörper aus Skizze mit **Ausdrücken in den Koordinaten** (`sketch.line` mit `"ri + 10"`) – folgt Parameteränderungen |
| V-05 | ✓ Fase |
| V-06 | ✓ `inspect.interference` meldet Überlappungen mit Volumen; Berührung zählt nicht als Überlappung |
| V-07 | ✓ `plastic.boss` (teilweise: Dom mit Bohrung); Rastnase/Nut-Feder nicht separat getestet |
| V-08 | ✓ Export STEP, 3MF, STL. **Einschränkung:** SolidCraft darf per MCP nur in seinen eigenen Temp-Ordner schreiben (`…\AppData\Local\Temp\solidcraft-mcp-…`); andere Ordner erst nach Start mit `--allow-path DIR`. Übertragung ins Repo daher über diesen Temp-Ordner |
| V-09 | ✓ Unabhängige Prüfung mit trimesh: STL und 3MF geschlossen (watertight), Normalen konsistent, Bounding Box exakt |
| V-10 | offen: `file.slicers` findet nur Cura 4.13.1, nicht PrusaSlicer. Nutzer slict selbst in PrusaSlicer aus der STL |

Erkenntnisse für die Modellierung: Eine Skizze auf der XZ-Ebene ohne `flip` hat die Skizzen-y-Achse nach **−Z**; mit `flip: true` zeigt sie nach +Z. `sketch.circle.center` akzeptiert nur Zahlen, die Parameterbindung erfolgt über `sketch.dimension` mit Ausdruck; Lochpositionen über `sketch.point` mit Ausdrücken.

## 3. Funktionstests (Protokoll-Vorlage)

Alle Tests in einem **neuen, separaten** Wegwerf-Design (vorher speichern lassen, falls beim Nutzer ein Design offen ist). Ergebnisse werden hier nachgetragen.

| Nr. | Test | Erfolgskriterium |
|---|---|---|
| V-02 | Parameter `d = 20 mm`, Zylinder über Skizze + Extrusion | Körper entsteht, `measure` bestätigt Ø 20 |
| V-03 | `set_parameter d = 30` | Neuberechnung ohne Fehler, Ø 30 |
| V-04 | Boolesche Operationen (Bohrung, Rippe), `solid.hole` | gültiger Volumenkörper |
| V-05 | Fasen/Rundungen | gelingt |
| V-06 | Zwei Körper, Interferenzprüfung mit absichtlicher Überlappung | Überlappung wird gemeldet |
| V-07 | `plastic.boss`, `plastic.snap_fit`, `plastic.lip` an einer Testschale | Features entstehen |
| V-08 | Export STEP, 3MF, STL in einen vereinbarten Ordner (Vorschlag: Projektordner des Nutzers, Unterordner `cad/test`) | Dateien vorhanden |
| V-09 | Unabhängige Netzprüfung der STL/3MF (Kopie in die Arbeitsumgebung, trimesh) | geschlossen, Maße ± 0,05 mm |
| V-10 | Übergabe an Cura (`file.save_mesh` mit `slicer`) und Slicen mit Kobra-2-Neo-Profil | G-Code ohne Fehler |

## 4. CAD-Workflow

1. **Parameter** aus 03 §4 als SolidCraft-Benutzerparameter mit Kommentar (Quelle/Datum); Export als CSV in den Projektordner.
2. **Teilemodelle** T1 Deckplatte, T2 Unterschale, T4 Sensorhalter, T6 Buchsenhalter (optional T5 Abdeckung) als Komponenten.
3. **Hüllkörper** (nicht druckbar) für Glasboden (Auflagering + Wölbung als Drehkörper aus M-09), LED-Ring, SuperMini, SW-420, USB-C-Buchse, Kabel.
4. **Baugruppe** mit Interferenz- und Abstandsprüfung, Schnittansicht durch Glas/Ring/Zentrierrand, Explosionsansicht, Stückliste.
5. **Prüfung vor Export** (03 §12) → Freigabepunkt G3 mit Screenshots.
6. **Export** je Teil: SolidCraft-Design, STEP, 3MF; Benennung `T1_deckplatte_v<Version>.3mf`.
7. **Unabhängige Netzprüfung** und **Slicer-Lauf** (§5).
8. **Testdrucke**: Toleranzcoupon → Zentrier-Testring (nur Auflage + Zentrierrand + Ring-Einfassung, ca. 7 mm hoch, druckt in < 1 h) → Elektronik-Passprobe → Vollteile.

## 5. Unabhängige Druckbarkeitsprüfung

Eine erfolgreiche CAD-Generierung gilt nicht als Nachweis der Druckbarkeit.

| Prüfung | Werkzeug | Kriterium |
|---|---|---|
| Netzgültigkeit | trimesh / admesh (Arbeitsumgebung) | geschlossen, mannigfaltig, konsistente Normalen |
| Maße | Bounding Box, `measure` | ± 0,05 mm zum Parameter |
| Wandstärke | Schnitte (`inspect.section`) | ≥ 1,6 mm |
| Überhänge | `inspect.draft` mit Druckrichtung | keine Flächen > 45° ohne Strategie |
| Slicer | Cura (beim Nutzer) mit Kobra-2-Neo-Profil | ohne Warnung; Stützmaterial = 0 für T1, T2, T4 |
| Passung | Coupon, Testring | reales Glas, Ring und Platinen passen |
