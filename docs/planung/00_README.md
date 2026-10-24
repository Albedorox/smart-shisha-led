# Smart Shisha LED-Untersetzer – Planungsdokumente (Phase A)

Stand: 2026-10-24 (Rev. 2: Messwerte, 24 LEDs, SolidCraft geprüft) · Grundlage: Lastenheft v1.0 · Status: **zur Prüfung, Phase B nicht freigegeben**

| Datei | Inhalt |
|---|---|
| 01_requirements.md | Anforderungen mit IDs und Abnahmekriterien, Abnahmecheckliste Phase A |
| 02_hardware.md | Komponenten, Pinout-Prüfung, Verdrahtung, Strombudget, Pegelwandler |
| 03_mechanical_design.md | Glasaufnahme, Parametersatz, Baugruppen, Toleranzen, Material, Montage |
| 04_firmware_architecture.md | Module, Datenmodelle, Zug-Zustandsmaschine, Farbverlauf, Fade, Robustheit |
| 05_webapp_specification.md | Technik, globale UI-Zustände, alle acht Screens im Detail |
| 06_api_contracts.md | Endpunkte, JSON-Schemas, Validierung, Fehlercodes, Polling |
| 07_ui_wireframes.md | Navigationskonzept und Wireframes aller Screens |
| 08_cad_mcp_plan.md | SolidCraft-Prüfung (über Rechner des Nutzers verfügbar, Katalog geprüft), Verifikationsprotokoll, CAD-Workflow |
| 09_test_plan.md | Hardware-, Firmware-, API-, UI-, Langzeit- und CAD-Tests |
| 10_open_questions.md | offene Fragen, Änderungsanträge CR-01…03, Entscheidungen D-01…D-31, Annahmen, Messaufgaben, Risiken |
| 11_implementation_plan.md | Arbeitspakete B0–B8, Abhängigkeiten, Freigabepunkte G0–G5 |

Einstieg: zuerst `10_open_questions.md` §1 (offene Fragen), dann 05/07 (Web-App) und 03 (Mechanik).

ID-Schema: Anforderungen `SYS/HW/SEN/LED/SES/WEB/API/MEC/SAF/DOC-nn`, Tests `T-…`, Entscheidungen `D-nn`, Annahmen `A-nn`, Messaufgaben `M-nn`, Risiken `R-nn`.

Fotos und Skizzen des Nutzers (Pinout, SW-420, LED-Ring, Glas): Ordner `fotos/`.
