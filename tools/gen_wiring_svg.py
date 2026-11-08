#!/usr/bin/env python3
"""Erzeugt docs/verdrahtung.svg (Verdrahtungsplan) aus den Bauteil- und Leitungslisten unten.

Aufruf aus dem Repo-Wurzelverzeichnis:  python3 tools/gen_wiring_svg.py
Mit --check wird nur geprüft, ob docs/verdrahtung.svg aktuell ist.
"""
import pathlib
import sys
from xml.sax.saxutils import escape

ROOT = pathlib.Path(__file__).resolve().parent.parent
DST = ROOT / "docs" / "verdrahtung.svg"

W, H = 1060, 740
FONT = "font-family='Helvetica, Arial, sans-serif'"
COLORS = {"5V": "#d6336c", "GND": "#212529", "3V3": "#e8590c", "DATA": "#2b8a3e", "SENS": "#1971c2"}
NET_LABEL = {"5V": "5 V", "GND": "GND", "3V3": "3,3 V", "DATA": "LED-Daten", "SENS": "Sensorsignal"}

# Bauteile: Name, Kasten (x, y, b, h), Untertitel, Pins (Name, x, y, Seite)
PARTS = [
    ("USB-C-Breakout", (20, 120, 190, 140), ["rotes Modul", "5,1 kΩ an CC", "nur VBUS und GND"],
     [("VBUS", 210, 160, "r"), ("GND", 210, 220, "r")]),
    ("ESP32-C3 SuperMini", (340, 160, 220, 400), ["Stiftreihe 5V … GPIO0", "ohne Stiftleisten löten"],
     [(n, 560, 200 + i * 45, "r") for i, n in
      enumerate(["5V", "GND", "3V3", "GPIO4", "GPIO3", "GPIO2", "GPIO1", "GPIO0"])]),
    ("LED-Ring 24 × WS2812", (780, 150, 230, 150), ["Pads auf der Rückseite", "DO bleibt frei"],
     [("5V", 780, 180, "l"), ("GND", 780, 220, "l"), ("DI", 780, 260, "l")]),
    ("SW-420", (780, 400, 230, 150), ["Vibrationssensor-Modul", "mit T4 an T1 geschraubt"],
     [("VCC", 780, 430, "l"), ("DO", 780, 470, "l"), ("GND", 780, 510, "l")]),
]
UNUSED = {"GPIO3", "GPIO2", "GPIO1"}

# Leitungen: Netz, Punkte (rechtwinklig), Verbindungspunkte
WIRES = [
    ("5V", [(210, 160), (240, 160), (240, 50), (740, 50)], []),
    ("GND", [(210, 220), (260, 220), (260, 95), (720, 95)], [(260, 95)]),
    ("GND", [(260, 220), (260, 700), (740, 700)], [(260, 220)]),
    ("5V", [(560, 200), (600, 200), (600, 50)], [(600, 50)]),
    ("GND", [(560, 245), (620, 245), (620, 95)], [(620, 95)]),
    ("5V", [(740, 50), (740, 180), (780, 180)], []),
    ("GND", [(720, 95), (720, 220), (780, 220)], []),
    ("DATA", [(560, 335), (680, 335), (680, 260), (780, 260)], []),
    ("3V3", [(560, 290), (640, 290), (640, 430), (780, 430)], []),
    ("SENS", [(560, 515), (700, 515), (700, 470), (780, 470)], []),
    ("GND", [(780, 510), (740, 510), (740, 700)], []),
]
NOTES = [
    (652, 368, ["ohne Pegelwandler (Anleitung);", "flackert der Ring: 74AHCT125", "oder 330 Ω in Reihe"]),
    (800, 330, ["Elko 470–1000 µF / ≥ 6,3 V", "direkt an die Ring-Pads, + an 5V"]),
]


def segs(points):
    return list(zip(points, points[1:]))


def hop_path(points, crossings):
    """Pfad mit kleinen Bögen an Kreuzungen waagerechter Abschnitte."""
    d = [f"M{points[0][0]},{points[0][1]}"]
    for (x1, y1), (x2, y2) in segs(points):
        if y1 == y2:
            xs = sorted((c for c in crossings if c[1] == y1 and min(x1, x2) < c[0] < max(x1, x2)),
                        key=lambda c: c[0], reverse=x2 < x1)
            step = 6 if x2 > x1 else -6
            for cx, _ in xs:
                d.append(f"L{cx - step},{y1} A6,6 0 0 {1 if step > 0 else 0} {cx + step},{y1}")
        d.append(f"L{x2},{y2}")
    return " ".join(d)


def crossings_of(i):
    """Kreuzungen der waagerechten Abschnitte von Leitung i mit senkrechten anderer Netze."""
    net, pts, _ = WIRES[i]
    out = []
    for (hx1, hy), (hx2, hy2) in segs(pts):
        if hy != hy2:
            continue
        for j, (net2, pts2, _) in enumerate(WIRES):
            if j == i or net2 == net:
                continue
            for (vx, vy1), (vx2, vy2) in segs(pts2):
                if vx == vx2 and min(hx1, hx2) < vx < max(hx1, hx2) and min(vy1, vy2) < hy < max(vy1, vy2):
                    out.append((vx, hy))
    return out


def text(x, y, s, size=13, anchor="start", weight="normal", fill="#212529"):
    return (f"<text x='{x}' y='{y}' {FONT} font-size='{size}' text-anchor='{anchor}' "
            f"font-weight='{weight}' fill='{fill}'>{escape(s)}</text>")


def dot(x, y, net):
    return f"<circle cx='{x}' cy='{y}' r='5' fill='{COLORS[net]}'/>"


def render() -> str:
    o = [f"<svg xmlns='http://www.w3.org/2000/svg' width='{W}' height='{H}' viewBox='0 0 {W} {H}'>",
         "<!-- Automatisch erzeugt durch tools/gen_wiring_svg.py – nicht von Hand ändern. -->",
         f"<rect width='{W}' height='{H}' fill='#ffffff'/>",
         text(40, 32, "Smart Shisha LED – Verdrahtung", 20, weight="bold")]
    o.append(text(1020, 32, "Kreuzung mit Bogen = keine Verbindung, Punkt = Verbindung", 12, "end",
                  fill="#495057"))
    # Leitungen zuerst, Bauteile darüber
    for i, (net, pts, dots) in enumerate(WIRES):
        cr = crossings_of(i)
        o.append(f"<path d='{hop_path(pts, cr)}' fill='none' stroke='{COLORS[net]}' stroke-width='3'/>")
        o += [dot(x, y, net) for x, y in dots]
    o.append(text(250, 44, "5-V-Schiene", 12, fill=COLORS["5V"]))
    o.append(text(270, 89, "Masse (GND)", 12, fill=COLORS["GND"]))
    o.append(text(270, 694, "Masse (GND)", 12, fill=COLORS["GND"]))
    # Elko an den Ring-Pads
    ex = 765
    o.append(dot(ex, 180, "5V") + dot(ex, 220, "GND"))
    o.append(f"<line x1='{ex}' y1='180' x2='{ex}' y2='195' stroke='#212529' stroke-width='2'/>"
             f"<line x1='{ex - 9}' y1='195' x2='{ex + 9}' y2='195' stroke='#212529' stroke-width='3'/>"
             f"<path d='M{ex - 9},205 Q{ex},199 {ex + 9},205' fill='none' stroke='#212529' stroke-width='3'/>"
             f"<line x1='{ex}' y1='203' x2='{ex}' y2='220' stroke='#212529' stroke-width='2'/>"
             + text(ex - 12, 194, "+", 12, "end", "bold"))
    for name, (x, y, w, h), sub, pins in PARTS:
        o.append(f"<rect x='{x}' y='{y}' width='{w}' height='{h}' rx='8' fill='#e9ecef' stroke='#495057' "
                 f"stroke-width='2'/>")
        # Titel neben den Pin-Beschriftungen mittig setzen
        cx = x + 60 + (w - 60) / 2 if pins[0][3] == "l" else x + (w - 60) / 2
        ty = y + h / 2 - 8 if len(pins) < 5 else y + 30
        o.append(text(cx, ty, name, 15, "middle", "bold"))
        for k, s in enumerate(sub):
            o.append(text(cx, ty + 20 + k * 16, s, 11, "middle", fill="#495057"))
        for pname, px, py, side in pins:
            grey = pname in UNUSED
            o.append(f"<circle cx='{px}' cy='{py}' r='6' fill='{'#ced4da' if grey else '#fff'}' "
                     f"stroke='{'#adb5bd' if grey else '#212529'}' stroke-width='2'/>")
            if side == "r":
                o.append(text(px - 12, py + 4, pname, 12, "end", fill="#adb5bd" if grey else "#212529"))
            else:
                o.append(text(px + 12, py + 4, pname, 12, fill="#adb5bd" if grey else "#212529"))
    o.append(text(450, 120, "USB-C des SuperMini: nur zum Programmieren,", 11, "middle", fill="#495057"))
    o.append(text(450, 134, "dann Powerbank am Breakout abziehen", 11, "middle", fill="#495057"))
    for x, y, lines in NOTES:
        for k, s in enumerate(lines):
            o.append(text(x, y + k * 15, s, 11, fill="#495057"))
    # Legende
    lx, ly = 40, 600
    o.append(text(lx, ly, "Leitungen", 13, weight="bold"))
    for k, net in enumerate(["5V", "GND", "3V3", "DATA", "SENS"]):
        yy = ly + 20 + k * 20
        o.append(f"<line x1='{lx}' y1='{yy - 4}' x2='{lx + 30}' y2='{yy - 4}' stroke='{COLORS[net]}' "
                 f"stroke-width='3'/>" + text(lx + 38, yy, NET_LABEL[net], 12))
    o.append("</svg>")
    return "\n".join(o) + "\n"


def main() -> int:
    svg = render()
    if "--check" in sys.argv:
        if not DST.exists() or DST.read_text(encoding="utf-8") != svg:
            print("docs/verdrahtung.svg ist veraltet: python3 tools/gen_wiring_svg.py ausführen")
            return 1
        print("docs/verdrahtung.svg ist aktuell")
        return 0
    DST.write_text(svg, encoding="utf-8")
    print(f"geschrieben: {DST.relative_to(ROOT)}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
