# Firmware flashen – direkt von GitHub, ohne Arduino IDE

GitHub baut die Firmware bei jeder Änderung auf `main` automatisch (Workflow „Firmware“) und legt sie als Datei `ShishaLed-merged.bin` im Release **firmware-latest** ab.

Du brauchst: Chrome oder Edge am Computer (nicht Firefox/Safari) und ein USB-C-Datenkabel.

1. **Datei holen:** Im Repository rechts auf **Releases** → **firmware-latest** → `ShishaLed-merged.bin` herunterladen.
   Direktlink: https://github.com/Albedorox/smart-shisha-led/releases/tag/firmware-latest
2. **Powerbank abziehen** und den SuperMini über die Programmieröffnung per USB mit dem Computer verbinden.
3. **Web-Flasher öffnen:** https://espressif.github.io/esptool-js/ (offizielles Werkzeug von Espressif).
4. **Connect** klicken, im Fenster den Eintrag mit „USB JTAG/serial debug unit“ (oder „ESP32-C3“) wählen.
   Klappt das nicht: am SuperMini **BOOT** gedrückt halten, kurz **RESET** drücken, BOOT loslassen, dann erneut Connect.
5. Bei **Flash Address** `0x0` eintragen, bei **File** die heruntergeladene `ShishaLed-merged.bin` wählen.
6. **Program** klicken und warten, bis 100 % erreicht sind (ca. 30 s).
7. **Disconnect**, USB abziehen, Powerbank wieder anschließen. Das WLAN **ShishaLed** erscheint, Web-App unter http://192.168.4.1.

Hinweise:

- Die Datei enthält Bootloader, Partitionstabelle und Firmware, darum Adresse `0x0`.
- Gespeicherte Einstellungen bleiben beim Flashen erhalten. Wer sie löschen will, klickt vorher im Web-Flasher auf **Erase Flash**.
- Für einen Pull Request liegt die gebaute Datei unter **Actions** → Lauf „Firmware“ → Abschnitt **Artifacts** (als ZIP, vor dem Flashen entpacken).
