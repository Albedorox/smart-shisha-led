#!/usr/bin/env python3
"""Erzeugt ShishaLed/web_page.h aus web/index.html (gzip als PROGMEM-Array + ETag).

Aufruf aus dem Repo-Wurzelverzeichnis:  python3 tools/gen_web_page.py
Mit --check wird nur geprüft, ob web_page.h aktuell ist (für die CI).
"""
import gzip
import hashlib
import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SRC = ROOT / "web" / "index.html"
DST = ROOT / "ShishaLed" / "web_page.h"


def render() -> str:
    html = SRC.read_bytes()
    # mtime=0, damit der Inhalt bei gleicher Quelle bitgleich bleibt
    gz = gzip.compress(html, compresslevel=9, mtime=0)
    etag = '"' + hashlib.sha1(html).hexdigest()[:16] + '"'
    lines = []
    for i in range(0, len(gz), 16):
        lines.append("  " + ",".join(f"0x{b:02x}" for b in gz[i:i + 16]) + ",")
    return (
        "// Automatisch erzeugt aus web/index.html durch tools/gen_web_page.py – nicht von Hand ändern.\n"
        f"// Quelle {len(html)} Byte, gzip {len(gz)} Byte.\n"
        "#pragma once\n\n"
        "#include <pgmspace.h>\n"
        "#include <stddef.h>\n"
        "#include <stdint.h>\n\n"
        f"static const char WEB_PAGE_ETAG[] = \"\\\"{etag[1:-1]}\\\"\";\n"
        f"static const size_t WEB_PAGE_GZ_LEN = {len(gz)};\n"
        "static const uint8_t WEB_PAGE_GZ[] PROGMEM = {\n"
        + "\n".join(lines)
        + "\n};\n"
    )


def main() -> int:
    out = render()
    if "--check" in sys.argv:
        if not DST.exists() or DST.read_text() != out:
            print("ShishaLed/web_page.h ist veraltet: python3 tools/gen_web_page.py ausführen", file=sys.stderr)
            return 1
        print("web_page.h aktuell")
        return 0
    DST.write_text(out)
    print(f"{DST.relative_to(ROOT)} geschrieben")
    return 0


if __name__ == "__main__":
    sys.exit(main())
