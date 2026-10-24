# 07 – Wireframes und Navigationskonzept

Stand: 2026-10-23 · Phase A · Referenzbreite 360 px (Smartphone hoch). Alle Zahlen in den Wireframes sind Platzhalter zur Darstellung des Layouts, keine Messwerte. Element- und Zustandsdetails: `05_webapp_specification.md`.

Legende: `[ Button ]` · `(•) ( )` Auswahl · `═══●═══` Slider · `▓` farbige Fläche · `◌` LED-Punkt · `›` Navigation · `⚠` Hinweis

## 1. Navigationskonzept

```
Tab-Leiste (immer sichtbar, unten)
┌────────┬────────┬────────┬──────────┬────────┐
│ Start  │ Licht  │Spieler │Rangliste │  Mehr  │
└────────┴────────┴────────┴──────────┴────────┘
                                          │
                                          ▼ (Bottom Sheet)
                              ┌───────────────────────┐
                              │ Farbverlauf         › │
                              │ Sensor              › │
                              │ System              › │
                              │ Hilfe               › │
                              └───────────────────────┘
```

| Screen | Route | Weg | Taps |
|---|---|---|---|
| 1 Dashboard | `#/start` | Tab | 1 |
| 2 Licht | `#/licht` | Tab | 1 |
| 6 Spieler | `#/spieler` | Tab (auch Chip im Dashboard) | 1 |
| 5 Rangliste | `#/rangliste` | Tab | 1 |
| 4 Farbverlauf | `#/farbverlauf` | Mehr › Farbverlauf (auch Link aus Licht) | 2 |
| 3 Sensor | `#/sensor` | Mehr › Sensor | 2 |
| 7 System | `#/system` | Mehr › System | 2 |
| 8 Hilfe | `#/hilfe` | Mehr › Hilfe (auch „?“ in Kopfzeile) | 1–2 |

Unterseiten aus „Mehr“ zeigen in der Kopfzeile einen Zurück-Pfeil; der Tab „Mehr“ bleibt markiert. Ab 900 px Breite ersetzt eine linke Seitenleiste mit allen acht Einträgen die Tab-Leiste.

## 2. Screen 1 – Dashboard

```
┌────────────────────────────────────┐
│ Shisha LED        ● Verbunden  ∿  ?│  Kopfzeile; ∿ = Puls-Symbol
│▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│  Zugstreifen in LED-Farbe (nur bei Zug)
├────────────────────────────────────┤
│  ┌──────────────────────────────┐  │
│  │ [Zug erkannt]   Zug von Alex │  │  Statuschip
│  │                              │  │
│  │          4,7 s               │  │  große Zahl
│  │                              │  │
│  │      ◌ ◌ ◌                   │  │
│  │    ◌       ◌                 │  │  LED-Vorschau (24 Punkte)
│  │    ◌       ◌                 │  │
│  │      ◌ ◌ ◌  …                │  │
│  │                              │  │
│  │ ▓▓▓▓▓▓▓▓▓▓▓▓▓|▓▓▓▓▓▓▓|▓▓▓▓   │  │  Verlaufsbalken
│  │ 0 s    ▲ 4,7  Gelb 3  Rot 6  │  │  Zeiger + Markierungen
│  │ Rot in 1,3 s                 │  │
│  └──────────────────────────────┘  │
│                                    │
│  Modus                             │
│  ┌──────────┬──────────┬────────┐  │
│  │ ● Smart  │Dauerlicht│  Aus   │  │  Segment-Schalter
│  └──────────┴──────────┴────────┘  │
│  Helligkeit 64/255 (25 %)        › │  → Licht
│                                    │
│  ┌──────────────────────────────┐  │
│  │ Letzter Zug                  │  │
│  │ Sam · 4,7 s · vor 2 min      │  │
│  │ 12 Züge in dieser Sitzung    │  │
│  │ [ Zur Rangliste › ]          │  │
│  └──────────────────────────────┘  │
│  Spieler: [ Alex ✎ ]               │  Chip → Spieler
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

Zustandsvarianten:

```
Bereit (Smart):                     Modus Aus:
│ [Bereit]                      │   │ [Bereit]                      │
│          –,– s                │   │ ⚠ LEDs aus – Messung läuft    │
│   ◌ ◌ ◌  (dunkel, Umriss)     │   │   weiter                      │

Fade-out:                           Verbindung verloren:
│ [Fade-out]  Zug beendet 4,7 s │   │▓ Verbindung zum Untersetzer  ▓│
│   ◌ ◌ ◌  (abnehmend)          │   │▓ verloren. WLAN prüfen.      ▓│
                                    │  Werte grau · Stand vor 6 s   │

Laden:
│ ░░░░░░░░   ░░░░░░  (Skelett)  │
```

## 3. Screen 2 – Licht

```
┌────────────────────────────────────┐
│ Licht                 ● Verbunden  │
├────────────────────────────────────┤
│  Modus                             │
│  ┌──────────┬──────────┬────────┐  │
│  │  Smart   │●Dauerlicht│  Aus  │  │
│  └──────────┴──────────┴────────┘  │
│  Feste Farbe, Messung läuft weiter │
│                                    │
│  Dauerlichtfarbe                   │
│  ┌──────────────────────────────┐  │
│  │ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ │  │  Vorschau (Farbe × Helligkeit)
│  │ #FFA03C                      │  │
│  └──────────────────────────────┘  │
│  (▓)(▓)(▓)(▓)(▓)(▓)(▓)(▓)          │  8 Farbvorschläge
│  R ════════════════●══  [255]      │
│  G ══════════●════════  [160]      │
│  B ════●══════════════  [ 60]      │
│  [ Farbwähler öffnen ]             │  natives input type=color
│                                    │
│  Helligkeit                        │
│  ════●════════════════  [ 64] 25 % │
│  ⚠ Helligkeit durch Stromgrenze    │  nur wenn limited
│    reduziert                       │
│                                    │
│  Aktuelle LED-Ausgabe   ◌◌◌◌◌◌     │  Realzustand
│                                    │
│  Farbgrenzen: Gelb ab 3,0 s ·      │
│  Rot ab 6,0 s   [ Anpassen › ]     │  → Farbverlauf
│                                    │
│  [ Beleuchtung auf Standard ]      │
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

Im Modus Smart/Aus: Farbbereich mit Hinweis „Wirkt nur im Modus Dauerlicht“ (nicht ausgeblendet).

## 4. Screen 3 – Sensor

```
┌────────────────────────────────────┐
│ ‹ Sensor              ● Verbunden  │
├────────────────────────────────────┤
│  ┌──────────────────────────────┐  │
│  │   ● ERSCHÜTTERUNG            │  │  bzw. ○ Ruhe
│  │   Letzter Impuls: vor 0,4 s  │  │
│  │   Zug aktiv · 2,1 s          │  │
│  └──────────────────────────────┘  │
│                                    │
│  Impulse (letzte 10 s): 23         │
│  ▁▃▇▅▁▁▂▇█▆▂▁▁▁▃▅▂▁▁▁              │  letzte 20 Abfragen
│  Abtastung alle 10 ms              │
│                                    │
│  Nachlaufzeit                      │
│  ═══════●═════════════  [1,00 s]   │
│  Zug endet nach 1,00 s ohne Impuls │
│                                    │
│  Diagnosemodus             [  ○ ]  │  Schalter
│  Züge zählen nicht für Rangliste   │
│                                    │
│  ┌ Sensorstatus ────────────────┐  │
│  │ ✓ Keine Auffälligkeiten      │  │  oder ⚠ Dauersignal / kein Signal
│  └──────────────────────────────┘  │
│  ▸ Tipps zu Position und           │
│    Empfindlichkeit                 │  aufklappbar
│                                    │
│  [ Sensor-Einstellungen auf Std. ] │
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

## 5. Screen 4 – Farbverlauf

```
┌────────────────────────────────────┐
│ ‹ Farbverlauf         ● Verbunden  │
├────────────────────────────────────┤
│  255┤▔▔▔▔▔▔▔▔▔╲         R ───      │
│     │  ╱      ╲         G - - -    │
│     │╱  R      ╲ G                 │  SVG-Kurve
│    0┼──────────●──────●──────── t  │
│     0 s       3,0 s  6,0 s         │
│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓  │  Farbband grün→gelb→rot
│                                    │
│  (●Grün 0 s) (●Gelb 3,0 s) (●Rot 6,0 s)
│                                    │
│  Zeit bis Gelb                     │
│  ════●════════════════  [3,0 s]    │
│  Zeit bis Rot                      │
│  ═══════●═════════════  [6,0 s]    │
│  ⚠ Rot muss mindestens 0,1 s nach  │  nur bei Fehler
│    Gelb erreicht werden.           │
│                                    │
│  Vorschau  ▓▓▓▓  2,4 s  [ ▶ ]      │  nur im Browser
│                                    │
│  Status: Entwurf – nicht gespeichert
│  [ Übernehmen ]  [ Verwerfen ]     │
│  [ Standardwerte (3 s / 6 s) ]     │
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

## 6. Screen 5 – Rangliste

```
┌────────────────────────────────────┐
│ Rangliste             ● Verbunden  │
├────────────────────────────────────┤
│  THE LONGEST PULL                  │
│  12 Züge erfasst                   │
│                                    │
│  ┌ Zug läuft: Alex · 3,1 s ──────┐ │  nur bei Zug
│  └───────────────────────────────┘ │
│  ┌──────────────────────────────┐  │
│  │ 1.  Sam            7,3 s     │  │
│  │     +12:08 · vor 3 min       │  │
│  ├──────────────────────────────┤  │
│  │ 2.  Alex           4,7 s     │  │
│  │     +27:41 · gerade eben     │  │
│  ├──────────────────────────────┤  │
│  │ 3.  Unbekannt      4,1 max.  │  │
│  │ 4.  …                        │  │
│  │ 5.  …                        │  │
│  └──────────────────────────────┘  │
│  ⓘ Wird beim Ausschalten gelöscht. │
│    Dauer inkl. Sensor-Nachlauf.    │
│                                    │
│  [ Leaderboard zurücksetzen ]      │  destruktiv (rot umrandet)
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘

Leerzustand:
│  ┌──────────────────────────────┐  │
│  │   Noch keine Züge in dieser  │  │
│  │   Sitzung. Der erste Zug     │  │
│  │   erscheint automatisch.     │  │
│  └──────────────────────────────┘  │
```

## 7. Screen 6 – Spieler

```
┌────────────────────────────────────┐
│ Spieler               ● Verbunden  │
├────────────────────────────────────┤
│  Aktueller Spieler                 │
│  ALEX                              │
│                                    │
│  Neuer Name                        │
│  ┌──────────────────────────────┐  │
│  │ Sam_                         │  │
│  └──────────────────────────────┘  │
│                              3/24  │
│  [ Übernehmen ]   [ Zurücksetzen ] │
│                                    │
│  ⓘ Gilt ab Beginn des nächsten     │
│    Zuges. Erfasste Züge bleiben    │
│    unverändert.                    │
│  ⚠ Gerade läuft ein Zug von Alex.  │  nur bei Zug
│                                    │
│  Zuletzt verwendet                 │
│  (Alex) (Sam) (Kim)                │  nur im Seitenspeicher
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

## 8. Screen 7 – System

```
┌────────────────────────────────────┐
│ ‹ System              ● Verbunden  │
├────────────────────────────────────┤
│  Gerät        Smart Shisha LED-…   │
│  WLAN         Shisha-LED-Control   │
│               offen, ohne Passwort │
│  IP-Adresse   192.168.4.1          │
│  Firmware     1.0.0 (Build …)      │
│  Webserver    läuft                │
│  LEDs         24                   │
│  Modus        Smart                │
│  Laufzeit     2 h 14 min           │
│                                    │
│  Aktualisierung (diese Seite)      │
│  (•)250 ms ( )500 ms ( )1 s        │
│                                    │
│  ⓘ Einstellungen, Namen und        │
│    Rangliste gehen beim Ausschalten│
│    verloren.                       │
│  ⓘ Jedes verbundene Gerät kann     │
│    Einstellungen ändern.           │
│                                    │
│  [ Alle Einstellungen auf Standard]│
│  Rangliste zurücksetzen? › Rangl.  │
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

## 9. Screen 8 – Hilfe

```
┌────────────────────────────────────┐
│ ‹ Hilfe                            │
├────────────────────────────────────┤
│  ▸ Verbinden                       │
│  ▾ Lichtmodi                       │
│    Smart: leuchtet nur beim Zug …  │
│    Dauerlicht: feste Farbe …       │
│    Aus: dunkel, Messung läuft …    │
│  ▸ Farbgrenzen                     │
│  ▸ Zugerkennung                    │
│  ▸ Nachlaufzeit                    │
│  ▸ Rangliste                       │
│  ▸ Datenverlust nach Neustart      │
│  ▸ Sicherheit und Pflege           │
│  Firmware 1.0.0                    │
├────────────────────────────────────┤
│ Start  Licht  Spieler  Rangliste ⋯ │
└────────────────────────────────────┘
```

## 10. Dialoge und Toasts

```
Bestätigungsdialog (Beispiel Rangliste):
┌────────────────────────────────────┐
│  Rangliste zurücksetzen?           │
│                                    │
│  Alle Einträge und der Zähler      │
│  werden gelöscht. Ein laufender    │
│  Zug wird danach normal gewertet.  │
│                                    │
│   [ Abbrechen ]   [ Zurücksetzen ] │  Abbrechen hat den Fokus
└────────────────────────────────────┘

Toast:      ┌──────────────────────┐
            │ ✓ Name übernommen    │   2 s, über der Tab-Leiste
            └──────────────────────┘
```

## 11. Breite ≥ 900 px

```
┌──────────────┬───────────────────────────────────────┐
│ Shisha LED   │  Dashboard                ● Verbunden │
│              │                                       │
│ › Start      │   (Inhalt wie mobil, max. 720 px,     │
│   Licht      │    Karten ggf. zweispaltig)           │
│   Spieler    │                                       │
│   Rangliste  │                                       │
│   Farbverl.  │                                       │
│   Sensor     │                                       │
│   System     │                                       │
│   Hilfe      │                                       │
└──────────────┴───────────────────────────────────────┘
```

## 12. Freigabe

Lastenheft §6.5 verlangt die Freigabe der Wireframes vor der Umsetzung. Diese Wireframes sind Teil der Phase-B-Freigabe (Freigabepunkt G0 in 11). Auf Wunsch kann vor G0 ein klickbarer, nicht produktiver Mock-up erstellt werden (D-24).
