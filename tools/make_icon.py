"""Generate the Wave Ordering app icon (1024x1024 PNG) for the iOS asset
catalog. Run with: python tools/make_icon.py
"""
from PIL import Image, ImageDraw

S = 4  # supersample factor for anti-aliasing
SIZE = 1024 * S

CREAM = (246, 243, 234, 255)   # #F6F3EA
NAVY = (22, 38, 63, 255)       # #16263F
RED = (192, 57, 43, 255)       # #C0392B

img = Image.new("RGBA", (SIZE, SIZE), CREAM)
draw = ImageDraw.Draw(img)

# --- Red sun (drawn first, dominant upper-right) ---
sun_cx, sun_cy, sun_r = 0.58 * SIZE, 0.36 * SIZE, 0.40 * SIZE
draw.ellipse([sun_cx - sun_r, sun_cy - sun_r, sun_cx + sun_r, sun_cy + sun_r], fill=RED)

# --- Navy wave (drawn on top, covers left/bottom, cuts into the sun) ---
wave_cx, wave_cy, wave_r = 0.20 * SIZE, 0.66 * SIZE, 0.40 * SIZE
draw.ellipse([wave_cx - wave_r, wave_cy - wave_r, wave_cx + wave_r, wave_cy + wave_r], fill=NAVY)

# --- Foam bumps along the crest of the wave (cream, on top of navy/red) ---
foam = [
    (0.34, 0.16, 0.075),
    (0.465, 0.115, 0.062),
    (0.575, 0.135, 0.050),
    (0.40, 0.265, 0.048),
    (0.495, 0.245, 0.040),
]
for fx, fy, fr in foam:
    cx, cy, r = fx * SIZE, fy * SIZE, fr * SIZE
    draw.ellipse([cx - r, cy - r, cx + r, cy + r], fill=CREAM)

# --- Shopping cart, cream, sitting on the red sun ---
cart_color = CREAM
lw = int(0.022 * SIZE)

# Basket (trapezoid, wider at top)
bx1, by1 = 0.535 * SIZE, 0.560 * SIZE   # top-left
bx2, by2 = 0.805 * SIZE, 0.560 * SIZE   # top-right
bx3, by3 = 0.755 * SIZE, 0.730 * SIZE   # bottom-right
bx4, by4 = 0.585 * SIZE, 0.730 * SIZE   # bottom-left
draw.polygon(
    [(bx1, by1), (bx2, by2), (bx3, by3), (bx4, by4)],
    outline=cart_color, width=lw,
)

# Handle (from top-left of basket up and to the left)
hx1, hy1 = bx1, by1
hx2, hy2 = 0.480 * SIZE, by1
hx3, hy3 = 0.430 * SIZE, 0.480 * SIZE
draw.line([(hx1, hy1), (hx2, hy2)], fill=cart_color, width=lw)
draw.line([(hx2, hy2), (hx3, hy3)], fill=cart_color, width=lw)

# Wheels
wheel_r = int(0.030 * SIZE)
for wx, wy in [(0.620 * SIZE, 0.790 * SIZE), (0.725 * SIZE, 0.790 * SIZE)]:
    draw.ellipse([wx - wheel_r, wy - wheel_r, wx + wheel_r, wy + wheel_r], fill=cart_color)

img = img.resize((1024, 1024), Image.LANCZOS).convert("RGB")
img.save("ios/Assets.xcassets/AppIcon.appiconset/icon-1024.png")
print("done")
