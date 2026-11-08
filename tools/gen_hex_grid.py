"""Generate the background hex-grid path data used in C++/Data/MainPage.xaml.

The skill nodes live on a lattice x = 960 + 156*c, y = 540 + 90*r (c + r even).
That lattice is exactly the centre set of a flat-top hexagon tiling with
circumradius R = 104 (1.5 * R = 156, sqrt(3) * R / 2 = 90), so the background
grid is drawn as that tiling. Prints a XAML path mini-language string.

Usage: python -I tools/gen_hex_grid.py
"""
import math

R = 104.0
W, H = 1920, 1080
CX, CY = 960.0, 540.0
STEP_X, STEP_Y = 156.0, 90.0


def corners(x, y):
    return [(x + R * math.cos(math.radians(60 * i)), y + R * math.sin(math.radians(60 * i))) for i in range(6)]


def key(p):
    return (round(p[0]), round(p[1]))


edges = set()
for c in range(-8, 9):
    for r in range(-9, 10):
        if (c + r) % 2:
            continue
        x, y = CX + STEP_X * c, CY + STEP_Y * r
        if x < -R or x > W + R or y < -R or y > H + R:
            continue
        pts = corners(x, y)
        for i in range(6):
            a, b = key(pts[i]), key(pts[(i + 1) % 6])
            edges.add((min(a, b), max(a, b)))

parts = ["M{:g},{:g}L{:g},{:g}".format(a[0], a[1], b[0], b[1]) for a, b in sorted(edges)]
print(" ".join(parts))
