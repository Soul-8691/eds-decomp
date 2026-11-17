"""Crop a folder of PNG frames (f00.png, f01.png, ...) and join them into a looping GIF.

  makegif.py FRAMES_DIR OUT.gif --crop X Y W H [--ms 1400] [--first-ms 1800] [--max-width 900]

Every frame gets the same crop; each is shown for --ms milliseconds (the first and last for --first-ms).
Colours are quantized per frame (adaptive 256-colour palette), and frames are not dithered so text stays crisp.
"""
import argparse
import glob
import os
from PIL import Image

ap = argparse.ArgumentParser()
ap.add_argument('frames')
ap.add_argument('out')
ap.add_argument('--crop', nargs=4, type=int, required=True)
ap.add_argument('--ms', type=int, default=1400)
ap.add_argument('--first-ms', type=int, default=1800)
ap.add_argument('--max-width', type=int, default=900)
ap.add_argument('--pattern', default='f*.png', help='frame files; b*.png for a capture burst')
ap.add_argument('--min-change', type=float, default=0.002,
                help='drop a frame whose crop differs from the last kept one in less than this fraction of pixels')
a = ap.parse_args()

from PIL import ImageChops

ap2 = argparse.ArgumentParser(add_help=False)
paths = sorted(glob.glob(os.path.join(a.frames, a.pattern)))
x, y, w, h = a.crop
images = [Image.open(p).convert('RGB').crop((x, y, x + w, y + h)) for p in paths]

def changed(p, q):
    diff = ImageChops.difference(p, q).convert('L').point(lambda v: 255 if v > 24 else 0)
    return diff.histogram()[255] >= a.min_change * w * h

stable = os.environ.get('STABLE') == '1'
crops = []
for i, im in enumerate(images):
    if stable and i + 1 < len(images) and changed(im, images[i + 1]):
        continue  # mid-animation (e.g. a panel fading): the next capture differs
    if crops and not changed(im, crops[-1]):
        continue  # same state (or just a blinking caret): keep the earlier frame
    crops.append(im)
frames = []
for im in crops:
    if im.width > a.max_width:
        im = im.resize((a.max_width, round(im.height * a.max_width / im.width)), Image.LANCZOS)
    frames.append(im.quantize(colors=256, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE))
durations = [a.ms] * len(frames)
durations[0] = durations[-1] = a.first_ms
os.makedirs(os.path.dirname(os.path.abspath(a.out)), exist_ok=True)
frames[0].save(a.out, save_all=True, append_images=frames[1:], duration=durations, loop=0, optimize=True, disposal=1)
print(f'{a.out}: {len(frames)} frames, {frames[0].width}x{frames[0].height}, {os.path.getsize(a.out) // 1024} KB')
