# 05 – Spezifikation der Webanwendung (acht Screens)

Stand: 2026-10-23 · Phase A · Wireframes und Navigation: `07_ui_wireframes.md` · Datenquellen: `06_api_contracts.md`

## 1. Umsetzungstechnik (zur Freigabe)

| Thema | Festlegung (Vorschlag) | Begründung |
|---|---|---|
| Architektur | Single-Page-Application in **einer** HTML-Datei mit eingebettetem CSS und JavaScript | eine Anfrage beim Laden, keine Abhängigkeiten |
| Framework | keines (Vanilla JS, ES2017) | Größe, keine externen Ressourcen |
| Routing | Hash-Routing (`#/start`, `#/licht`, `#/spieler`, `#/rangliste`, `#/farbverlauf`, `#/sensor`, `#/system`, `#/hilfe`) | funktioniert ohne serverseitige Routen; Zurück-Taste des Browsers funktioniert |
| Auslieferung | gzip-komprimiert aus dem Flash | Ziel ≤ 30 KB komprimiert |
| Grafiken | Inline-SVG und CSS (Verlaufsbalken, Kurve, Farbkreis) | keine Bilddateien, keine Grafikbibliothek |
| Fonts | Systemschrift (`system-ui, -apple-system, Segoe UI, Roboto, sans-serif`), Zahlen mit `font-variant-numeric: tabular-nums` | keine externen Fonts |
| Texte | Deutsch, zentrale Texttabelle im Script | spätere Übersetzung möglich |
| Ausgabe von Nutzertexten | ausschließlich über `textContent` (nie `innerHTML`) | verhindert Skript-Einschleusung über Spielernamen |
| Zielbrowser | iOS Safari ≥ 15, Android Chrome ≥ 100, Samsung Internet, Firefox Android (aktuell) | gängige Smartphones |
| Lokale Speicherung im Browser | keine | Lastenheft: keine Persistenz |

## 2. Navigation (Kurzfassung, Details in 07)

Untere Tab-Leiste mit fünf Einträgen: **Start · Licht · Spieler · Rangliste · Mehr**. „Mehr“ öffnet eine Liste mit **Farbverlauf · Sensor · System · Hilfe**. Jeder Screen ist mit höchstens zwei Taps erreichbar. Zusätzlich verlinken Screens untereinander (z. B. Licht → Farbverlauf).

Begründung der Abweichung von acht gleichwertigen Einträgen: Acht Tabs ergeben auf 360 px Breite unter 45 px je Eintrag und unlesbare Beschriftungen. Die vier häufig genutzten Screens liegen direkt in der Leiste, die vier Einstellungs- und Info-Screens unter „Mehr“ (Lastenheft §6.3 erlaubt das ausdrücklich).

## 3. Globale UI-Zustände

| Zustand | Auslöser | Darstellung | Verhalten |
|---|---|---|---|
| Initiales Laden | Seite geöffnet, noch keine Statusantwort | Skelett-Platzhalter (graue Balken) statt Werten, Statuspunkt grau „Verbinde …“ | Bedienelemente deaktiviert |
| Verbindung aktiv | letzte Statusantwort < 1 s alt | Statuspunkt grün „Verbunden“ in der Kopfzeile | alles bedienbar |
| Statusabfrage fehlgeschlagen | 1 Fehlschlag (Timeout 1,5 s, HTTP-Fehler, ungültiges JSON) | Statuspunkt gelb „Verbindung instabil“ | Werte bleiben sichtbar |
| Verbindung verloren | 2 Fehlschläge in Folge **oder** letzte gültige Antwort > 2 s alt | rotes Banner oben: „Verbindung zum Untersetzer verloren. Mit WLAN *Shisha-LED-Control* verbunden?“; Live-Werte ausgegraut mit Hinweis „Stand vor X s“ | Schreibende Bedienelemente deaktiviert; automatische Wiederverbindung mit Backoff; nach Erfolg Banner weg und Toast „Wieder verbunden“ |
| Ungültige Eingabe | clientseitige Prüfung oder 400 vom Server | Feld rot umrandet, Fehlertext direkt unter dem Feld (Text aus `error.message`) | nichts wird gesendet bzw. übernommen; vorheriger Wert bleibt aktiv |
| Änderung erfolgreich | 200 auf POST | Toast unten (2 s) „Gespeichert“ bzw. spezifisch („Modus: Dauerlicht“) | Werte aus Server-Antwort übernehmen |
| Sendefehler | POST-Timeout/5xx | Toast rot „Änderung nicht übernommen – erneut versuchen“ mit Button „Wiederholen“ | Eingabefeld behält den gewünschten Wert, Anzeige „nicht gespeichert“ |
| Sensor aktiv | `sensor.levelActive` | kleines Puls-Symbol in der Kopfzeile blinkt einmal je Impuls | – |
| Zug aktiv | `phase = pull` | Kopfzeile farbig in aktueller LED-Farbe (schmaler Streifen), Statuschip „Zug erkannt“ | – |
| Fade-out aktiv | `phase = fade` | Statuschip „Fade-out“ | – |
| Leeres Leaderboard | `entries = []` | Leerzustand (siehe Screen 5) | – |
| Konfiguration zurückgesetzt | 200 auf Reset | Toast „Standardwerte wiederhergestellt“ | alle Formulare neu befüllt |
| Konfiguration extern geändert | `configVersion` gestiegen, ohne eigene Änderung | dezenter Hinweis „Einstellungen wurden auf einem anderen Gerät geändert“ | Formulare aktualisieren, außer dem Feld, das gerade bearbeitet wird |
| Stromgrenze aktiv | `led.limited = true` | Hinweis am Helligkeitsregler „Helligkeit durch Stromgrenze reduziert“ | – |

**Regel gegen veraltete Daten:** Jeder Live-Wert trägt intern den Zeitstempel der Antwort. Ist er älter als 2 s, wird er ausgegraut und mit „Stand vor X s“ markiert; laufende Zähler (Zugdauer) werden dann nicht weitergezählt.

**Schreibverhalten:** Slider senden erst beim Loslassen (`change`), während des Ziehens nur lokale Vorschau; zusätzlich höchstens alle 300 ms ein Zwischenwert (Drosselung) für die Helligkeit, damit die LEDs „mitgehen“. Laufende Anfragen werden nicht parallel gestellt: eine neue Änderung ersetzt eine noch wartende.

## 4. Responsive Darstellung

| Breite | Layout |
|---|---|
| 320–479 px (Smartphone hoch) | einspaltig, Tab-Leiste unten (fix), Inhalte mit 16 px Rand |
| 480–899 px (Smartphone quer, kleine Tablets) | einspaltig, max. Inhaltsbreite 560 px zentriert; Tab-Leiste unten |
| ≥ 900 px | Navigation als linke Seitenleiste mit allen acht Einträgen, Inhalte max. 720 px |

- Touch-Ziele ≥ 44 × 44 px, Abstand ≥ 8 px.
- `viewport-fit=cover` und `env(safe-area-inset-bottom)` für Geräte mit Home-Indikator.
- Kein horizontales Scrollen; lange Namen werden mit Auslassungspunkten gekürzt (voller Name per Tap).
- Schriftgrößen in `rem`, Systemvergrößerung bis 200 % ohne Überlappung.

## 5. Designsystem

| Token | Wert (Vorschlag) | Verwendung |
|---|---|---|
| `--bg` | #0E1012 | Hintergrund |
| `--surface` | #181B1F | Karten |
| `--surface-2` | #22262B | Eingabefelder, Tab-Leiste |
| `--text` | #ECEFF3 | Haupttext (Kontrast > 15 : 1) |
| `--text-muted` | #9AA3AD | Nebentext (Kontrast ≈ 7 : 1) |
| `--accent` | #4FC3F7 | Fokus, aktive Navigation (neutral, kollidiert nicht mit den Zugfarben) |
| `--pull-green` / `--pull-yellow` / `--pull-red` | #00FF00 / #FFFF00 / #FF0000 als LED-Darstellung; für Text/Flächen gedämpft #3DDC84 / #FFD54F / #FF5252 | funktionale Zugfarben |
| `--danger` | #FF5252 | Fehler, destruktive Aktionen |
| Radius | 12 px Karten, 999 px Chips | |
| Abstände | 4/8/12/16/24 px | |
| Große Zahl | 56 px, `tabular-nums` | laufende Zugdauer |

Animationen nur: Farbübergang der LED-Vorschau, Fortschrittsbalken, Puls-Symbol. `prefers-reduced-motion` schaltet diese auf sofortige Wechsel.

## 6. Gemeinsame Komponenten

| Komponente | Beschreibung |
|---|---|
| Kopfzeile | Produktname „Shisha LED“, Statuspunkt + Text, Puls-Symbol, Zugstreifen in LED-Farbe |
| Statuschip | „Bereit“ (grau), „Zug erkannt“ (in Zugfarbe), „Fade-out“ (grau, abnehmend) |
| Moduswahl | Segment-Schalter mit drei Feldern: Smart · Dauerlicht · Aus |
| LED-Vorschau | Kreis aus 24 Punkten (SVG), gefüllt mit der aktuellen logischen Farbe × Helligkeit; bei „aus“ dunkle Punkte mit Umriss |
| Slider mit Zahlenfeld | Range-Input plus numerische Anzeige; Tippen auf die Zahl öffnet ein Zahlenfeld zur exakten Eingabe |
| Bestätigungsdialog | modales Overlay mit Titel, Text, „Abbrechen“ (Standardfokus) und destruktivem Button |
| Toast | unten über der Tab-Leiste, 2 s, Screenreader-Ankündigung (`aria-live="polite"`) |

## 7. Screen-Spezifikationen

Für jeden Screen: Zweck, Elemente mit Datenquelle, Zustände, Interaktionen, Validierung, Fehlerfälle. Alle Live-Daten stammen aus `/api/status` (S), `/api/leaderboard` (L) oder Antworten von POST-Anfragen.

---

### Screen 1 – Dashboard / Live-Status (`#/start`)

**Zweck:** zentrale Übersicht über den aktuellen Betriebszustand.

| Element | Inhalt / Datenquelle | Format |
|---|---|---|
| Kopfzeile | Produktname, Verbindungsstatus (Client) | siehe §6 |
| Statuschip | `phase` (S) | Bereit / Zug erkannt / Fade-out |
| Laufende Zugdauer | `pull.elapsedMs` (S); zwischen zwei Antworten clientseitig weitergezählt (max. 500 ms Vorlauf, dann Stillstand bis zur nächsten Antwort) | „4,7 s“ groß; im Ruhezustand „–,– s“ |
| Spieler des laufenden Zuges | `pull.player` (S) | „Zug von Alex“ |
| LED-Vorschau | `led.r/g/b`, `led.effectiveBrightness`, `led.on` (S) | Ring aus 24 Punkten |
| Fortschritt Gelb/Rot | `pull.elapsedMs`, `config.yellowMs`, `config.redMs` | horizontaler Balken mit Farbverlauf grün→gelb→rot, Markierungen bei Gelb und Rot, Zeiger auf aktueller Dauer; Beschriftung „Gelb in 0,6 s“ / „Rot erreicht“ |
| Aktuelle Helligkeit | `config.brightness`, `led.limited` | „Helligkeit 64/255 (25 %)“ |
| Letzter Zug | `session.lastPull` (S) | „Letzter Zug: Sam · 4,7 s · vor 2 min“; „max.“-Kennzeichen bei `truncated`; „(Diagnose)“ bei `diagnostic` |
| Zähler | `session.pullCount` (S) | „12 Züge in dieser Sitzung“ |
| Moduswahl | `config.mode` | Segment-Schalter |
| Schnellzugriffe | Links zu Licht und Rangliste; aktueller Spieler als Chip → Spieler-Screen | Karten/Chips |
| Hinweisfläche | Diagnosemodus aktiv; Sensor-Dauersignal; Stromgrenze | gelbe Infokarten, nur wenn zutreffend |

**Zustände:** Laden (Skelett), Bereit, Zug aktiv, Fade-out, Modus Dauerlicht (Fortschrittsbalken bleibt sichtbar, da Züge weiter gemessen werden; Hinweis „LEDs zeigen Dauerlicht“), Modus Aus (Hinweis „LEDs aus – Messung läuft weiter“), Verbindung verloren (Werte ausgegraut).

**Interaktionen:** Moduswechsel → `POST /api/config {"mode": …}`; Schalter zeigt sofort optimistisch den neuen Modus mit Ladeindikator, bei Fehler Rücksprung + Toast. Tap auf Rangliste/Licht/Spieler → Navigation.

**Validierung:** keine Eingaben außer Moduswahl (feste Werte).

**Fehlerfälle:** POST fehlgeschlagen → Rücksprung auf Servermodus; `phase` unbekannt → Anzeige „Unbekannter Zustand“ und Log in der Konsole.

---

### Screen 2 – Beleuchtungssteuerung (`#/licht`)

**Zweck:** Einstellung der Lichtmodi und visuellen Parameter.

| Element | Inhalt / Datenquelle | Format |
|---|---|---|
| Moduswahl | `config.mode` | Segment-Schalter + Erklärung je Modus (eine Zeile) |
| RGB-Farbwähler (Dauerlicht) | `config.steadyColor` | drei Slider R/G/B (0–255) mit Zahlenfeldern + 8 Farbvorschläge (Warmweiß, Weiß, Rot, Orange, Gelb, Grün, Cyan, Blau); zusätzlich `<input type="color">` als Komfortwähler, sofern vom Browser unterstützt |
| Farbvorschau | lokal berechnet aus gewählter Farbe × Helligkeit | große Fläche + Hex-Wert |
| Helligkeitsregler | `config.brightness` 0–255 | Slider + Zahl + Prozent |
| Aktuelle LED-Farbe | `led.*` (S) | LED-Vorschau-Ring (zeigt Realzustand, nicht die Auswahl) |
| Zeitgrenzen | `config.yellowMs`, `config.redMs` | „Gelb ab 3,0 s · Rot ab 6,0 s“ + Link „Farbverlauf anpassen“ |
| Stromgrenzen-Hinweis | `led.limited` | Infozeile |
| Zurücksetzen | – | Button „Beleuchtung auf Standard“ |

**Zustände:** Farbwähler ist in den Modi Smart/Aus sichtbar, aber mit Hinweis „wirkt nur im Modus Dauerlicht“ (Farbe kann vorab eingestellt werden). Helligkeit 0 → Hinweis „Helligkeit 0: LEDs bleiben dunkel“.

**Interaktionen:**
- Farbwahl: Vorschlag-Tap sendet sofort; Slider senden beim Loslassen (`{"steadyColor": {...}}`).
- Helligkeit: gedrosselt alle 300 ms während des Ziehens, final beim Loslassen.
- Moduswechsel wie Screen 1.
- Zurücksetzen → Bestätigungsdialog „Modus, Helligkeit und Dauerlichtfarbe auf Standard setzen?“ → `{"reset": "lighting"}`.

**Validierung:** Zahlenfelder nur Ganzzahlen 0–255; außerhalb → Feldfehler „Wert zwischen 0 und 255 eingeben“, nichts senden.

**Fehlerfälle:** 400 → Feldfehler mit Servertext; Sendefehler → Toast + Wert als „nicht gespeichert“ markiert. Der Screen setzt keine bestimmte LED-Hardware voraus: die Vorschau zeigt die logische Ausgabe, mit dem Hinweis im Hilfetext, dass die reale Darstellung von den LEDs abhängt.

---

### Screen 3 – Zugerkennung und Sensor-Kalibrierung (`#/sensor`)

**Zweck:** Prüfung und Einstellung des SW-420.

| Element | Inhalt / Datenquelle | Format |
|---|---|---|
| Sensorzustand | `sensor.levelActive` | großer Indikator „Erschütterung“ (gelb) / „Ruhe“ (grau); Pegel aktualisiert mit Polling (kurze Impulse sind evtl. nicht sichtbar – Hinweistext) |
| Letzter Impuls | `sensor.lastPulseAgoMs`, `sensor.hasPulse` | „vor 0,4 s“ / „noch keiner seit Start“ |
| Zug aktiv | `pull.active`, `pull.elapsedMs` | Chip + Dauer |
| Nachlaufzeit | `config.tailMs` | Slider 200–5.000 ms, Raster 50, Anzeige in s mit 2 Nachkommastellen |
| Impulse im Beobachtungsfenster | `sensor.pulsesInWindow`, `sensor.windowMs` | „23 Impulse in den letzten 10 s“ + Mini-Balkenreihe der letzten 20 Abfragen (clientseitig aus Status-Historie) |
| Abtastintervall | `sensor.sampleIntervalMs` | „Abtastung alle 10 ms“ (nur Anzeige) |
| Diagnosemodus | `config.diagnostic` | Schalter „Diagnosemodus (Züge zählen nicht für die Rangliste)“ |
| Sensorstatus | `sensor.fault`, `sensor.hasPulse` | „Dauersignal erkannt – Empfindlichkeit am Poti verringern oder Montage prüfen“ / „Noch keine Erschütterung erkannt – Empfindlichkeit erhöhen oder Verkabelung prüfen“ / „Keine Auffälligkeiten“ |
| Hinweise | statisch | Positionierung, Poti-Einstellung, typische Fehltrigger (Abstellen, Tischstoß) |
| Zurücksetzen | – | „Sensor-Einstellungen auf Standard“ |

**Zustände:** normal; Dauersignal (rote Karte); kein Signal seit Start (gelbe Karte); Diagnosemodus aktiv (gelber Rahmen um den Screen + Hinweis auf Dashboard).

**Interaktionen:** Slider → `{"tailMs": n}` beim Loslassen; Diagnoseschalter → `{"diagnostic": bool}`; Reset → Bestätigung → `{"reset": "sensor"}`.

**Validierung:** `tailMs` 200–5.000, Raster 50 (Slider erzwingt; Zahlenfeld prüft).

**Fehlerfälle:** wie global. Es wird kein „Selbsttest“ angeboten oder behauptet.

---

### Screen 4 – Farbverlauf konfigurieren (`#/farbverlauf`)

**Zweck:** Konfiguration des zeitabhängigen Farbverlaufs.

| Element | Inhalt / Datenquelle | Format |
|---|---|---|
| Regler „Zeit bis Gelb“ | `config.yellowMs` | Slider 0,5–30,0 s, Raster 0,1 s |
| Regler „Zeit bis Rot“ | `config.redMs` | Slider 0,6–60,0 s, Raster 0,1 s |
| Verlaufskurve | lokal aus Entwurfswerten | SVG: x-Achse Zeit 0 … `redMs·1,25`, darüber ein Farbband grün→gelb→rot; drei Stützpunkte als Kreise mit Beschriftung; Linien für R- und G-Kanal (0–255) |
| Stützpunktfarben | fest | Chips „0 s Grün (0,255,0)“, „Gelb (255,255,0)“, „Rot (255,0,0)“ mit den aktuellen Zeiten |
| Vorschau | lokal | Button „Vorschau abspielen“: animiert die Farbe von 0 bis `redMs + 1 s` in Echtzeit, Zeitanzeige; nur im Browser, LEDs bleiben unverändert |
| Status | – | „Entwurf – nicht gespeichert“ oder „Aktiv“ |
| Buttons | – | „Übernehmen“ (primär), „Verwerfen“, „Standardwerte (3 s / 6 s)“ |

**Zustände:** Aktiv (Entwurf = Server), Entwurf geändert, Entwurf ungültig, wird gesendet.

**Interaktionen:** Regler ändern nur den Entwurf (sofortige Validierung und Kurvenaktualisierung). „Übernehmen“ sendet `{"yellowMs", "redMs"}` gemeinsam (damit die Reihenfolgeprüfung nie an einem Zwischenzustand scheitert). Standardwerte → Bestätigung → `{"reset": "gradient"}`.

Hinweis zur Abweichung vom Wortlaut „unmittelbare Anwendung“: Zwei voneinander abhängige Werte werden gemeinsam übernommen, um ungültige Zwischenzustände zu vermeiden. Alternative: automatisches Senden 500 ms nach der letzten gültigen Änderung (D-23).

**Validierung (clientseitig, identisch zum Server):**
- Gelb 500–30.000 ms, Rot 600–60.000 ms, Raster 100.
- `redMs ≥ yellowMs + 100`, sonst Fehlermeldung unter dem Rot-Regler: „Rot muss mindestens 0,1 s nach Gelb erreicht werden.“ und „Übernehmen“ deaktiviert.
- Verschiebt der Nutzer Gelb über Rot hinaus, wird Rot **nicht** automatisch mitgezogen (keine versteckten Änderungen); es erscheint die Fehlermeldung.

**Fehlerfälle:** 400 `validation_failed` → Meldung am Feld; Sendefehler → Entwurf bleibt erhalten.

---

### Screen 5 – Leaderboard / The Longest Pull (`#/rangliste`)

**Zweck:** Anzeige der fünf längsten Züge der laufenden Sitzung.

| Element | Inhalt / Datenquelle | Format |
|---|---|---|
| Titel | – | „The Longest Pull“ |
| Liste | `entries[]` (L) | Zeile: Rang (1–3 mit Medaillen-Farbakzent, als Text „1.“), Spielername, Dauer „7,3 s“ (eine Nachkommastelle, `durationMs/1000` gerundet), Sitzungszeitpunkt „+12:08 seit Start“ und darunter klein „vor 3 min“ (D-22) |
| Kennzeichen | `truncated` | „max.“ hinter der Dauer |
| Zähler | `pullCount` (L) | „12 Züge erfasst“ |
| Laufender Zug | `pull` (S) | Banner „Zug läuft: Alex · 3,1 s“; ab dem Moment, in dem die Dauer den 5. Platz übertreffen würde: „… auf Rekordkurs“ |
| Hinweis | statisch | „Die Rangliste wird beim Ausschalten gelöscht. Die Dauer enthält die Nachlaufzeit des Sensors.“ |
| Reset | – | Button „Leaderboard zurücksetzen“ (destruktiv, unten) |

**Zustände:** Laden; Leerzustand („Noch keine Züge in dieser Sitzung. Der erste Zug erscheint hier automatisch.“); 1–5 Einträge; Verbindung verloren (Liste ausgegraut, „Stand vor X s“).

**Interaktionen:** Live-Aktualisierung über `lbVersion`; neuer Eintrag wird 2 s hervorgehoben. Reset → Dialog „Rangliste und Zähler löschen? Ein laufender Zug wird danach normal gewertet.“ → `POST /api/leaderboard/reset {"confirm": true}` → Toast „Rangliste zurückgesetzt“.

**Validierung:** keine Eingaben.

**Fehlerfälle:** Leaderboard-Abruf fehlgeschlagen → letzte Liste bleibt mit Hinweis, erneuter Versuch beim nächsten Status. Sortierung erfolgt nur serverseitig; der Client sortiert nicht um.

---

### Screen 6 – Spielerprofil / Aktueller Spieler (`#/spieler`)

**Zweck:** Festlegung des Namens für den nächsten Zug.

| Element | Inhalt / Datenquelle | Format |
|---|---|---|
| Aktiver Name | `player.current` (S) | groß „Aktueller Spieler: Alex“ |
| Eingabefeld | lokal | `<input type="text" autocomplete="off" autocapitalize="words" enterkeyhint="done">`, Platzhalter „Name eingeben“ |
| Zeichenzähler | lokal | „5/24“, gezählt in Codepoints (`Array.from(str).length`) |
| Buttons | – | „Übernehmen“ (primär), „Zurücksetzen“ (setzt auf „Unbekannt“) |
| Hinweis | statisch | „Der Name wird beim Beginn des nächsten Zuges übernommen. Bereits erfasste Züge ändern sich nicht.“ |
| Laufender Zug | `pull.active`, `pull.player` | „Aktuell läuft ein Zug von Sam – dein neuer Name gilt ab dem nächsten Zug.“ |
| Zuletzt verwendete Namen | lokal, nur im Arbeitsspeicher der Seite | bis zu 5 Chips zum schnellen Wechseln (gehen beim Neuladen verloren) |

**Zustände:** unverändert; geändert (Übernehmen aktiv); ungültig; gesendet; Name „Unbekannt“ (Hinweis „Kein Name gesetzt“).

**Interaktionen:** Übernehmen oder Enter → `POST /api/name`; Zurücksetzen → `{"name": ""}` (keine Bestätigung nötig, da harmlos). Erfolg → Toast „Name übernommen“ bzw. „Gilt ab dem nächsten Zug“ bei `appliesFromNextPull`.

**Validierung (clientseitig, identisch zum Server):** > 24 Codepoints → Fehler „Maximal 24 Zeichen“, Übernehmen deaktiviert; Steuerzeichen werden beim Einfügen entfernt und mit Hinweis gemeldet; leer/Leerzeichen → Hinweis „Leerer Name wird als ‚Unbekannt‘ gespeichert“ (erlaubt). Hinweis: Manche Emoji bestehen aus mehreren Codepoints und zählen entsprechend mehrfach – der Zähler zeigt das ehrlich an.

**Fehlerfälle:** 400 `invalid_name` → Servertext am Feld.

---

### Screen 7 – System / Einstellungen (`#/system`)

**Zweck:** Systeminformationen und Zurücksetzen von Einstellungen.

| Element | Inhalt / Datenquelle |
|---|---|
| Gerätename | `system.deviceName` |
| WLAN-SSID | `system.ssid` + „offen, ohne Passwort“ |
| IP-Adresse | `system.ip` |
| Firmware-Version / Build | `system.fwVersion`, `system.build` |
| Webserver-Status | abgeleitet: „läuft“, solange Statusantworten eintreffen; sonst „nicht erreichbar“ (ehrlich: der Client kann nur die Erreichbarkeit beurteilen) |
| Anzahl LEDs | `system.ledCount` |
| Betriebsmodus | `config.mode` |
| Laufzeit seit Start | `uptimeMs` → „2 h 14 min“ |
| Verbundene Geräte | `system.clients` – nur wenn vom Server geliefert, sonst Zeile ausgeblendet |
| Aktualisierungsintervall | Client-Einstellung 250 / 500 / 1.000 ms (nur diese Seite, bis zum Neuladen) |
| Hinweis flüchtige Speicherung | „Alle Einstellungen, Namen und die Rangliste gehen beim Ausschalten verloren.“ |
| Hinweis Zugriff | „Jedes verbundene Gerät kann Einstellungen ändern.“ |
| Konfiguration zurücksetzen | Button „Alle Einstellungen auf Standard“ (setzt Modus, Helligkeit, Farbe, Zeitgrenzen, Nachlauf, Diagnose zurück; **nicht** Rangliste und Spielername) |
| Leaderboard zurücksetzen | separater Link „Zur Rangliste …“ (bewusst nicht hier, um Verwechslung zu vermeiden) |
| Neustart | **nicht enthalten** (D-14); erst nach Spezifikation und Freigabe |

**Interaktionen:** Reset → Dialog „Alle Einstellungen auf Standardwerte setzen? Rangliste und Spielername bleiben erhalten.“ → `{"reset": "all"}`.

**Fehlerfälle:** fehlende Felder in der Antwort → Zeile zeigt „nicht verfügbar“ (keine erfundenen Werte). Freier Heap und WLAN-Signalstärke werden nicht angezeigt.

---

### Screen 8 – Hilfe / Informationen (`#/hilfe`)

**Zweck:** Erklärung der Bedienung und Einschränkungen. Statisch, ohne Serverdaten (außer Anzeige der IP-Adresse aus der Konstante).

Inhalt in aufklappbaren Abschnitten (`<details>`):

1. **Verbinden:** WLAN *Shisha-LED-Control* wählen (offen), Browser öffnen, `192.168.4.1` eingeben. Hinweis: Manche Smartphones melden „Kein Internet“ – Verbindung trotzdem halten; ggf. mobile Daten kurz deaktivieren, falls die Seite nicht lädt.
2. **Lichtmodi:** Smart (leuchtet nur beim Zug), Dauerlicht (feste Farbe), Aus (dunkel, Messung läuft weiter).
3. **Farbgrenzen:** Grün → Gelb → Rot, mit den aktuellen Zeiten.
4. **Zugerkennung:** Der Sensor erkennt Erschütterungen (Blubbern, Vibrationen), nicht den Luftstrom. Abstellen des Glases oder Stöße können Züge auslösen.
5. **Nachlaufzeit:** Wie lange nach der letzten Erschütterung der Zug noch als aktiv gilt; sie ist in der gemessenen Zugdauer enthalten.
6. **Rangliste:** Top 5 der Sitzung, gelöscht bei Neustart oder Reset.
7. **Datenverlust:** Nach dem Ausschalten sind alle Einstellungen und die Rangliste weg.
8. **Sicherheit und Pflege:** nur 5-V-USB-Netzteil verwenden; Kabel nicht unter Zug; Untersetzer nicht in Wasser tauchen, nur feucht abwischen; keine Kohle oder heißen Teile auf den Untersetzer; offenes WLAN – jedes verbundene Gerät kann Einstellungen ändern.
9. **Version:** Firmware-Version (aus letzter Statusantwort, falls verfügbar).

Der Screen funktioniert auch bei verlorener Verbindung (bereits geladen).

## 8. Barrierefreiheit

- Semantische Elemente (`<nav>`, `<main>`, `<button>`, `<label>`), Formulare mit Labels.
- Farbe nie alleiniger Informationsträger: Zugstatus immer auch als Text.
- Fokus sichtbar (`--accent`-Umrandung); Dialoge fangen den Fokus.
- `aria-live` für Statuswechsel („Zug erkannt“, „Zug beendet: 4,7 s“), gedrosselt auf Zustandswechsel, nicht jede Aktualisierung.

## 9. Ressourcenbudget

| Größe | Ziel |
|---|---|
| HTML+CSS+JS gzip | ≤ 30 KB |
| Erstes Laden über AP | < 2 s |
| DOM-Aktualisierung pro Statusantwort | nur geänderte Werte; keine komplette Neuzeichnung |
| Speicher im Browser | Status-Historie für Mini-Balken max. 20 Einträge |
