#!/usr/bin/env python3
"""
Generate a simple cFd icon set for macOS.
Requires: Pillow  (pip install Pillow)
Usage: python3 gen_icon.py <iconset_dir>
"""
import sys, os

try:
    from PIL import Image, ImageDraw, ImageFont
except ImportError:
    print("Pillow not found — skipping icon generation")
    sys.exit(0)

sizes = [16, 32, 64, 128, 256, 512, 1024]
out_dir = sys.argv[1] if len(sys.argv) > 1 else "cFd.iconset"
os.makedirs(out_dir, exist_ok=True)

BG    = (15,  20,  30)   # dark navy
GREEN = (63, 185, 80)    # GitHub-style green
CYAN  = (88, 166, 255)   # accent blue

def make_icon(size):
    img  = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    r    = int(size * 0.12)   # corner radius

    # Rounded rect background
    draw.rounded_rectangle([0, 0, size-1, size-1], radius=r, fill=BG)

    # Draw "cFd" text centred
    font_size = max(8, int(size * 0.38))
    try:
        font = ImageFont.truetype("DejaVuSansMono-Bold.ttf", font_size)
    except Exception:
        try:
            font = ImageFont.truetype("/System/Library/Fonts/Menlo.ttc", font_size)
        except Exception:
            font = ImageFont.load_default()

    text = "cFd"
    bbox = draw.textbbox((0, 0), text, font=font)
    tw, th = bbox[2] - bbox[0], bbox[3] - bbox[1]
    tx = (size - tw) // 2 - bbox[0]
    ty = (size - th) // 2 - bbox[1]

    # Draw each letter in a different color
    c_w, _ = draw.textbbox((0, 0), "c", font=font)[2:4]
    f_w, _ = draw.textbbox((0, 0), "F", font=font)[2:4]
    d_w, _ = draw.textbbox((0, 0), "d", font=font)[2:4]
    # fall back to simple single-color render if bboxes are wonky
    draw.text((tx, ty), "c",  fill=GREEN, font=font)
    draw.text((tx + c_w, ty), "F", fill=CYAN,  font=font)
    draw.text((tx + c_w + f_w, ty), "d", fill=(220,220,220), font=font)

    return img

for s in sizes:
    img = make_icon(s)
    img.save(os.path.join(out_dir, f"icon_{s}x{s}.png"))
    if s <= 512:
        img_2x = make_icon(s * 2)
        img_2x = img_2x.resize((s * 2, s * 2), Image.LANCZOS)
        img_2x.save(os.path.join(out_dir, f"icon_{s}x{s}@2x.png"))

print(f"Generated icon set in {out_dir}/")
