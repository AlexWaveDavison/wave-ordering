"""Generate the small UI icons used in the app (tab bar, search, empty
states). Drawn in the brand navy on transparent; Qt Quick Controls tints them
(e.g. white on the tab bar) via `icon.color`.

Run with: python tools/make_ui_icons.py
"""
from pathlib import Path

from PIL import Image, ImageDraw

OUT = Path(__file__).resolve().parent.parent / "qml" / "icons"
SIZE = 96            # 24dp at 4x; scaled down smoothly by Qt
S = 4                # supersampling factor for anti-aliasing
NAVY = (22, 38, 63, 255)
W = int(7.5 * S)     # stroke width (~1.9dp)


def canvas():
    img = Image.new("RGBA", (SIZE * S, SIZE * S), (0, 0, 0, 0))
    return img, ImageDraw.Draw(img)


def save(img, name):
    img.resize((SIZE, SIZE), Image.LANCZOS).save(OUT / name)


def p(v):
    """Position on a 24-unit grid -> supersampled pixels."""
    return v * SIZE * S / 24


def catalogue():
    img, d = canvas()
    for x, y in [(3.5, 3.5), (13.5, 3.5), (3.5, 13.5), (13.5, 13.5)]:
        d.rounded_rectangle([p(x), p(y), p(x + 7), p(y + 7)], radius=p(1.8), outline=NAVY, width=W)
    save(img, "catalogue.png")


def basket():
    img, d = canvas()
    # handle
    d.line([p(1.5), p(4), p(4.5), p(4), p(6.8), p(15), p(18.5), p(15), p(20.8), p(7.5), p(5.6), p(7.5)],
           fill=NAVY, width=W, joint="curve")
    # wheels
    for cx in (8.3, 17):
        r = p(1.7)
        d.ellipse([p(cx) - r, p(19.5) - r, p(cx) + r, p(19.5) + r], fill=NAVY)
    save(img, "basket.png")


def orders():
    img, d = canvas()
    d.rounded_rectangle([p(5), p(2.5), p(19), p(21.5)], radius=p(2), outline=NAVY, width=W)
    for y, x2 in [(8, 15.5), (12, 15.5), (16, 12.5)]:
        d.line([p(8.5), p(y), p(x2), p(y)], fill=NAVY, width=W)
    save(img, "orders.png")


def search():
    img, d = canvas()
    r = p(6.2)
    cx, cy = p(10.2), p(10.2)
    d.ellipse([cx - r, cy - r, cx + r, cy + r], outline=NAVY, width=W)
    d.line([p(14.8), p(14.8), p(20.5), p(20.5)], fill=NAVY, width=W)
    save(img, "search.png")


def logo():
    src = Path(__file__).resolve().parent.parent / "ios" / "Assets.xcassets" / "AppIcon.appiconset" / "icon-1024.png"
    big = Image.open(src).convert("RGBA").resize((512, 512), Image.LANCZOS)
    mask = Image.new("L", big.size, 0)
    ImageDraw.Draw(mask).rounded_rectangle([0, 0, 511, 511], radius=112, fill=255)
    big.putalpha(mask)
    big.resize((128, 128), Image.LANCZOS).save(OUT / "logo.png")


if __name__ == "__main__":
    OUT.mkdir(parents=True, exist_ok=True)
    catalogue()
    basket()
    orders()
    search()
    logo()
