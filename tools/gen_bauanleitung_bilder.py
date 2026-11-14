#!/usr/bin/env python3
"""Erzeugt die Bilder der Bauanleitung (docs/bauanleitung/*.png) aus den STL-Dateien.

Aufruf aus dem Repo-Wurzelverzeichnis:  python3 tools/gen_bauanleitung_bilder.py
Benötigt numpy, trimesh und matplotlib. Elektronikbauteile sind vereinfachte Quader
an den Positionen aus dem CAD-Modell (cad/v3/parameter.csv, Hüllkörper im Skript).
"""
import pathlib

import matplotlib
import numpy as np
import trimesh

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402
from matplotlib.collections import PolyCollection  # noqa: E402

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "docs" / "bauanleitung"
T1 = trimesh.load(ROOT / "cad/v6/stl/T1_Deckplatte_v6.stl")
T2 = trimesh.load(ROOT / "cad/v3/stl/T2_Unterschale_v3.stl")
T4 = trimesh.load(ROOT / "cad/v1/stl/T4_Distanzring_v1.stl")

# Farben: Druckteile hell, Elektronik kräftig, Markierungen gut lesbar auf Weiß
C_T1, C_T2, C_T4 = "#e9e4da", "#cfc8bb", "#f2a33a"
C_PCB_BLUE, C_PCB_RED, C_METAL, C_LED = "#2f5fa8", "#c0392b", "#b8bec6", "#f4f4f4"
M2, M3, SEN, GREY, TXT = "#138a92", "#d9822b", "#d6336c", "#5c6370", "#212529"


# Lage der Bohrungen und Mulden (aus den STL-Dateien gemessen)
M2_PTS = [(x, y) for x in (-32.3, 32.3) for y in (-24.8, 24.8)]
M3_PTS = [(x, y) for x in (-33.2, 33.2) for y in (-33.2, 33.2)]
SEN_PT = (0, -47)
SLOT = (0, 39.7)
FEET = [(50 * np.cos(a), 50 * np.sin(a)) for a in np.radians([0, 90, 180, 270])]
DRAIN = [(-21, 51), (21, -51)]


# ---------- Geometrie der Bauteile (Lage wie im CAD, T1 unten bei z 12) ----------
def box(x0, x1, y0, y1, z0, z1):
    b = trimesh.creation.box(extents=[x1 - x0, y1 - y0, z1 - z0])
    b.apply_translation([(x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2])
    return b


MCU_X0, MCU_X1 = -58.3, -35.6   # Platinenkanten: USB-Seite (mcu_x0) und Antennenseite


def supermini():
    return [(box(MCU_X0, MCU_X1, -9.05, 9.05, 3.5, 5.1), C_PCB_BLUE),        # Platine
            (box(MCU_X0 - 1.3, MCU_X0 + 7.7, -4.5, 4.5, 5.1, 8.3), C_METAL),  # USB-C
            (box(-49.0, -43.0, -3.0, 3.0, 5.1, 6.0), "#30343b")]             # ESP32-C3


def breakout_screws():
    parts = []
    for y in (-8, 8):
        c = trimesh.creation.cylinder(radius=2.2, height=1.2)
        c.apply_translation([57.6, y, 5.7])
        parts.append((c, C_METAL))
    return parts


def breakout():
    return [(box(47.6, 60.1, -10.7, 10.7, 3.5, 5.1), C_PCB_RED),
            (box(53.0, 61.5, -4.5, 4.5, 5.1, 8.3), C_METAL)]


def sensor():
    """SW-420 hängend unter T1: Bauteile nach unten, Stiftleiste zur Mitte (−X)."""
    y0, y1 = -47 - 7.1, -47 + 7.1
    return [(box(-6.8, 24.4, y0, y1, 8.9, 10.5), C_PCB_BLUE),
            (box(2.0, 9.0, -51.0, -43.0, 4.5, 8.9), "#3a6fd0"),               # Poti
            (box(11.0, 20.0, -50.0, -44.0, 5.5, 8.9), C_METAL),               # Federkontakt
            (box(-14.8, -6.8, -50.0, -44.0, 9.2, 10.2), "#d4af37")]           # Stiftleiste


def ring_pcb():
    a = trimesh.creation.annulus(r_min=35.9, r_max=43.0, height=1.6)
    a.apply_translation([0, 0, 16.4 + 0.8])
    return [(a, "#1d1f24")]


def ring_top():
    """LEDs und die vier M2-Schraubenköpfe."""
    parts = []
    for x, y in M2_PTS:
        c = trimesh.creation.cylinder(radius=1.9, height=1.3)
        c.apply_translation([x, y, 18.65])
        parts.append((c, C_METAL))
    for k in range(24):
        ang = 2 * np.pi * k / 24
        led = box(-2.5, 2.5, -2.5, 2.5, 18.0, 19.6)
        led.apply_transform(trimesh.transformations.rotation_matrix(ang, [0, 0, 1]))
        led.apply_translation([39.45 * np.cos(ang), 39.45 * np.sin(ang), 0])
        parts.append((led, C_LED))
    return parts


def ring():
    return ring_pcb() + ring_top()


def lift(parts, dz):
    out = []
    for m, c in parts:
        m = m.copy()
        m.apply_translation([0, 0, dz])
        out.append((m, c))
    return out


# ---------- Darstellung ----------
def cam(d, up=(0, 0, 1)):
    d = np.array(d, float)
    d /= np.linalg.norm(d)
    f = -d
    u = np.array(up, float)
    u = u - f * (u @ f)
    u /= np.linalg.norm(u)
    return f, u, np.cross(f, u)


def hexrgb(h):
    return np.array([int(h[i:i + 2], 16) / 255 for i in (1, 3, 5)])


class View:
    def __init__(self, d, up=(0, 0, 1), size=(8, 6.5), light=(0.4, -0.5, 1.0)):
        self.f, self.u, self.r = cam(d, up)
        self.L = np.array(light, float) / np.linalg.norm(light)
        self.fig, self.ax = plt.subplots(figsize=size)
        self.fig.patch.set_facecolor("white")
        self.pts = []
        self.layer = 0

    def p(self, pt):
        pt = np.array(pt, float)
        return pt @ self.r, pt @ self.u

    def draw(self, *layers):
        """Jede Ebene wird für sich nach Tiefe sortiert und über die vorherige gemalt."""
        for parts in layers:
            self._draw(parts)

    def _draw(self, parts):
        polys, cols, depth = [], [], []
        for m, color in parts:
            tri = m.vertices[m.faces]
            n = m.face_normals
            vis = n @ self.f < 0
            tri, n = tri[vis], n[vis]
            shade = 0.5 + 0.5 * np.clip(n @ self.L, 0, 1) + 0.1 * np.clip(n @ (-self.f), 0, 1)
            polys.append(np.stack([tri @ self.r, tri @ self.u], axis=-1))
            cols.append(np.clip(hexrgb(color)[None, :] * shade[:, None], 0, 1))
            depth.append((tri @ self.f).mean(axis=1))
        P, C, D = np.concatenate(polys), np.concatenate(cols), np.concatenate(depth)
        o = np.argsort(-D)
        self.layer += 1
        self.ax.add_collection(PolyCollection(P[o], facecolors=C[o], edgecolors=C[o], linewidths=0.3,
                                              zorder=self.layer / 10))
        self.pts.append(P.reshape(-1, 2))
        allp = np.concatenate(self.pts)
        self.ax.set_aspect("equal")
        self.ax.axis("off")
        pad = 24
        self.ax.set_xlim(allp[:, 0].min() - pad, allp[:, 0].max() + pad)
        self.ax.set_ylim(allp[:, 1].min() - 6, allp[:, 1].max() + 6)

    def ring_at(self, pt, color, r=3.6, lw=2.6):
        x, y = self.p(pt)
        self.ax.add_patch(plt.Circle((x, y), r, fill=False, ec=color, lw=lw, zorder=5))

    def label(self, pt, text, dx, dy, color=TXT, size=12.5, ha="center"):
        x, y = self.p(pt)
        self.ax.annotate(text, xy=(x, y), xytext=(x + dx, y + dy), color=color, fontsize=size,
                         weight="bold", ha=ha, va="center", zorder=7,
                         bbox=dict(boxstyle="round,pad=0.25", fc="white", ec="none", alpha=0.85),
                         arrowprops=dict(arrowstyle="-", color=color, lw=1.6, shrinkA=0, shrinkB=3))

    def zoom(self, x0, x1, y0, y1, z0, z1, pad=8):
        pts = np.array([self.p((x, y, z)) for x in (x0, x1) for y in (y0, y1) for z in (z0, z1)])
        self.ax.set_xlim(pts[:, 0].min() - pad, pts[:, 0].max() + pad)
        self.ax.set_ylim(pts[:, 1].min() - pad, pts[:, 1].max() + pad)

    def save(self, name):
        self.fig.savefig(OUT / f"{name}.png", dpi=120, facecolor="white", bbox_inches="tight",
                         pad_inches=0.08)
        plt.close(self.fig)
        print("geschrieben:", (OUT / f"{name}.png").relative_to(ROOT))


ISO = (0.55, -0.95, 0.85)


def bild_uebersicht():
    """Nummern wie in der Tabelle „Auf einen Blick“."""
    v = View((0.3, -1, 0.95), size=(8, 9))
    v.draw([(T2, C_T2)], supermini() + breakout(), lift([(T4, C_T4)], 22), lift(sensor(), 22),
           lift([(T1, C_T1)], 46), lift(ring_pcb(), 46), lift(ring_top(), 46))
    v.label((-43, 0, 65.6), "1  LED-Ring", -30, 22)
    v.label((52, 30, 66), "2  T1 Deckplatte", 26, 18)
    v.label((14, -47, 28), "3  SW-420 (hängt unter T1)", 40, -22, SEN)
    v.label((-2, -47, 31), "4  T4 Distanzring (verdeckt)", 6, -44, M3)
    v.label((-47, 0, 5.1), "5  ESP32-C3 SuperMini", -20, 22, C_PCB_BLUE)
    v.label((56, 4, 5.1), "6  USB-C-Breakout", 26, 10, C_PCB_RED)
    v.label((-44, -40, 8), "7  T2 Unterschale", -26, -10)
    v.save("00_uebersicht")


def bild_t1_oben():
    v = View((0, -0.35, 1))
    v.draw([(T1, C_T1)])
    for x, y in M2_PTS:
        v.ring_at((x, y, 16.4), M2)
    v.label((32.3, -24.8, 16.4), "4 × M2-Einschmelzmutter\nvon oben, bündig", 30, -40, M2)
    x, y = v.p((*SLOT, 16.4))
    v.ax.add_patch(plt.Rectangle((x - 8.5, y - 4.6), 17, 9.2, fill=False, ec=GREY, lw=2.2, ls="--", zorder=5))
    v.label((0, 44, 16.4), "Kabelaussparung", 0, 20, GREY)
    v.save("01_t1_oben_m2")


def bild_t1_unten():
    v = View((0, 0, -1), up=(0, 1, 0), light=(0.3, 0.4, -1))
    v.draw([(T1, C_T1)])
    for x, y in M3_PTS:
        v.ring_at((x, y, 12), M3, r=4.3)
    v.ring_at((*SEN_PT, 12), SEN, r=4.3)
    v.label((33.2, 33.2, 12), "4 × M3 Gehäuse", -24, 18, M3)
    v.label((32.3, -24.8, 12), "M2-Durchgang\n(nichts setzen)", -40, -2, GREY, size=10.5)
    v.label((0, -47, 12), "1 × M3 Sensor", 0, 20, SEN)
    x, y = v.p((*SLOT, 12))
    v.ax.add_patch(plt.Rectangle((x - 8.5, y - 4.6), 17, 9.2, fill=False, ec=GREY, lw=2.2, ls="--", zorder=5))
    v.label((0, 35, 12), "Kabelaussparung", 0, -16, GREY)
    v.ax.text(*v.p((0, 58, 12)), "T1 von unten gesehen", ha="center", va="center", fontsize=11, color=GREY)
    v.save("02_t1_unten_m3")


def bild_ring():
    v = View(ISO)
    v.draw([(T1, C_T1)], ring_pcb(), ring_top())
    v.label((0, 43, 18.2), "Pads über der Aussparung", 34, 24, GREY)
    v.label((32.3, -24.8, 19.3), "4 × M2 × 6", 40, -14, M2)
    v.label((-39.5, 0, 19.6), "LEDs nach oben", -24, 18)
    v.save("03_ring")


def bild_sensor():
    v = View((0, 0, -1), up=(0, 1, 0), light=(0.3, 0.4, -1), size=(9, 4.2))
    v.draw([(T1, C_T1)], [(T4, C_T4)], sensor())
    v.label((-10.8, -47, 9.2), "Stiftleiste zur SuperMini-Seite:\nKabel VCC, GND, DO", 36, 20)
    v.label((5.5, -48, 4.5), "Poti", 12, -17, "#3a6fd0")
    v.label((15.5, -47, 5.5), "Federkontakt", -22, -17, GREY)
    v.label((0, -45, 12), "T4 zwischen Platine und T1,\nM3 × 6 von unten", -44, 22, M3)
    v.ax.annotate("", xy=v.p((0, -24, 12)), xytext=v.p((0, -32, 12)),
                  arrowprops=dict(arrowstyle="-|>", color=GREY, lw=1.6))
    v.ax.text(*v.p((0, -21, 12)), "zur Gehäusemitte", ha="center", va="center", fontsize=10.5, color=GREY)
    v.ax.text(*v.p((0, -71, 12)), "T1 von unten gesehen (Ausschnitt)", ha="center", va="center", fontsize=10.5,
              color=GREY)
    v.ax.set_xlim(-80, 80)
    v.ax.set_ylim(-75, -14)
    v.save("04_sensor")


def bild_t2_plaetze():
    v = View((0.0, -0.6, 1))
    v.draw([(T2, C_T2)])
    x, y = v.p(((MCU_X0 + MCU_X1) / 2, 0, 3.5))
    v.ax.add_patch(plt.Rectangle((x - 12.2, y - 8.2), 24.4, 16.4, fill=False, ec=C_PCB_BLUE, lw=2.4, ls="--",
                                 zorder=5))
    v.label((MCU_X0, 0, 3.5), "5  Rahmen SuperMini", -2, 40, C_PCB_BLUE)
    v.label((-62.5, 0, 7), "Programmier-Öffnung", -4, -34, C_PCB_BLUE)
    for yy in (-8, 8):
        v.ring_at((57.6, yy, 3.5), C_PCB_RED, r=3.4)
    v.label((57.6, 8, 3.5), "6  zwei Sockel Breakout", 0, 40, C_PCB_RED)
    v.label((62.5, 0, 7), "USB-C-Öffnung", 2, -34, C_PCB_RED)
    for xx, yy in M3_PTS:
        v.ring_at((xx, yy, 12), GREY, r=4.6, lw=2)
    v.label((33.2, 33.2, 12), "Schraubdome", -26, 20, GREY)
    v.label((15, -47, 10.5), "Sensorstifte", -38, -18, SEN)
    v.save("05_t2_plaetze")


def bild_supermini_detail():
    v = View((0.45, -0.75, 1), size=(8, 5.2))
    v.draw([(T2, C_T2)], supermini())
    v.zoom(-63, -30, -14, 14, 0, 9, pad=6)
    v.label((MCU_X0 - 1.3, 0, 6.7), "USB-C zur Programmier-Öffnung", -10, 16, C_PCB_BLUE, size=11.5)
    v.label((MCU_X1 + 0.6, 0, 5.75), "Schnappzunge (Antennenseite)", 14, -14, GREY, size=11.5)
    v.label((-46, -9.05, 5.1), "Bauteilseite oben", -6, -16, C_PCB_BLUE, size=11.5)
    v.save("05a_supermini")


def bild_breakout_detail():
    v = View((-0.45, -0.75, 1), size=(8, 5.2))
    pcb, usb = breakout()
    v.draw([(T2, C_T2)], [pcb], [usb], breakout_screws())
    v.zoom(44, 63, -15, 15, 0, 9, pad=6)
    v.label((57.6, -8, 6.3), "2 × Blechschraube 2,5 × 4\nin die Sockel (ohne Mutter)", -14, -16, C_PCB_RED,
            size=11.5)
    v.label((60.5, 0, 8.3), "USB-C-Buchse oben,\nzeigt nach außen in die Wandöffnung", 2, 14, C_PCB_RED, size=11.5)
    v.label((60.1, 9, 4.3), "Platinenkante in der Wandtasche", -16, -4, GREY, size=11.5, ha="right")
    v.save("05b_breakout")


def bild_t2_elektronik():
    v = View((0.0, -0.6, 1))
    v.draw([(T2, C_T2)], supermini() + breakout(), breakout_screws())
    v.label((MCU_X0 - 1.3, 0, 7), "5  SuperMini", -4, 34, C_PCB_BLUE)
    v.label((61.5, 0, 7), "6  Breakout", 4, 34, C_PCB_RED)
    v.label((15, -38.6, 10.5), "Sensorstifte", -38, -22, SEN)
    v.save("06_t2_elektronik")


def bild_t2_unten():
    v = View((0, 0, -1), up=(0, 1, 0), light=(0.3, 0.4, -1))
    v.draw([(T2, C_T2)])
    for x, y in M3_PTS:
        v.ring_at((x, y, 0), M3, r=4.6)
    for x, y in FEET:
        v.ring_at((x, y, 0), GREY, r=6.2, lw=2.2)
    v.label((-33.2, 33.2, 0), "4 × M3 × 12", 26, 22, M3)
    v.label((-50, 0, 0), "4 × Silikonfuß Ø10", 30, 10, GREY)
    v.label((21, -51, 0), "Ablauf", 12, -12, GREY)
    v.ax.text(*v.p((0, 70, 0)), "T2 von unten gesehen", ha="center", va="center", fontsize=11, color=GREY)
    v.save("07_t2_unten")


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    bild_uebersicht()
    bild_t1_oben()
    bild_t1_unten()
    bild_ring()
    bild_sensor()
    bild_t2_plaetze()
    bild_supermini_detail()
    bild_breakout_detail()
    bild_t2_elektronik()
    bild_t2_unten()
