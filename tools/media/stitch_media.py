#!/usr/bin/env python3
"""The README's stitched media (README_TODO.md), made with ffmpeg from the captures in screenshots/:

    python3 tools/media/stitch_media.py [duel-speed] [trailer] [highlights]      (all three when none is named)

  screenshots/videos/duel-speed.mp4   the same turn at DEFAULT, ALWAYS and SUPER side by side (each labelled; the
                                      shorter ones hold their last frame)
  screenshots/videos/trailer.mp4      about 45 s: the title screen, the duelist select, a pre-duel screen, rock-paper-
                                      scissors, a Synchro Summon, a Link Summon, the CPU's Magic Cylinder, a pack
  screenshots/gifs/highlights.gif     five short clips: a format duelist, a Synchro Summon, a pack opening, Deck
                                      Recipes, the CPU's Trap

Run the capture scripts first (run_tests.ps1 -Media). ffmpeg: $FFMPEG, else on PATH, else the newest
~/Downloads/ffmpeg-*/bin/ffmpeg.exe (as tools/testing_scenarios/lib_eds.lua finds it).
"""
import glob
import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
SHOTS = ROOT / 'screenshots'
GIFS = SHOTS / 'gifs'
VIDEOS = SHOTS / 'videos'
FONT = 'C\\:/Windows/Fonts/consolab.ttf'       # (drawtext: the colon escaped)


def ffmpeg():
    exe = os.environ.get('FFMPEG') or shutil.which('ffmpeg')
    if not exe:
        found = sorted(glob.glob(str(Path.home() / 'Downloads' / 'ffmpeg-*' / '**' / 'bin' / 'ffmpeg.exe'),
                                 recursive=True))
        exe = found[-1] if found else None
    if not exe:
        sys.exit('ffmpeg not found: set FFMPEG')
    return exe


def run(args):
    cmd = [ffmpeg(), '-y', '-hide_banner', '-loglevel', 'error'] + args
    subprocess.run(cmd, check=True)


def duel_speed():
    modes = ['default', 'always', 'super']
    inputs, parts = [], []
    for i, m in enumerate(modes):
        inputs += ['-i', str(VIDEOS / f'duel-speed-{m}.mp4')]
        parts.append(f"[{i}:v]scale=480:320:flags=neighbor,drawtext=fontfile='{FONT}':text='{m.upper()}':"
                     f"x=8:y=8:fontsize=22:fontcolor=white:borderw=3:bordercolor=black[v{i}]")
    graph = ';'.join(parts) + ';[v0][v1][v2]hstack=inputs=3[out]'
    out = VIDEOS / 'duel-speed.mp4'
    run(inputs + ['-filter_complex', graph, '-map', '[out]', '-pix_fmt', 'yuv420p', '-c:v', 'libx264', str(out)])
    return out


# (source, start in s or None, length in s or None); a negative start counts from the end; a .png is held `length` s
TRAILER = [
    (SHOTS / 'Title Screen.png', None, 2.5),
    (SHOTS / 'Duelists (Screen 1).png', None, 2.0),
    (GIFS / 'pre-duel.gif', None, None),
    (GIFS / 'rps.gif', None, None),
    (GIFS / 'synchro.gif', -7.0, None),
    (GIFS / 'link.gif', -10.0, None),
    (VIDEOS / 'cpu-play.mp4', None, None),
    (GIFS / 'pack-pull.gif', -6.0, None),
]

HIGHLIGHTS = [
    (GIFS / 'pre-duel.gif', 1.0, 1.4),
    (GIFS / 'synchro.gif', -3.2, 1.4),
    (GIFS / 'pack-pull.gif', -4.0, 1.4),
    (SHOTS / 'Deck Recipes.png', None, 1.2),
    (VIDEOS / 'cpu-play.mp4', 4.5, 2.2),
]


def segment_inputs(segments):
    args = []
    for src, start, length in segments:
        if not Path(src).exists():
            sys.exit(f'missing {src}: run its capture script first')
        if str(src).endswith('.png'):
            args += ['-loop', '1', '-t', str(length), '-i', str(src)]
            continue
        if start is not None and start < 0:
            args += ['-sseof', str(start)]
        elif start is not None:
            args += ['-ss', str(start)]
        if length is not None:
            args += ['-t', str(length)]
        args += ['-i', str(src)]
    return args


def concat(segments, width, height, fps, out, gif):
    n = len(segments)
    parts = [f'[{i}:v]scale={width}:{height}:flags=neighbor,fps={fps},setsar=1,format=rgb24[s{i}]' for i in range(n)]
    graph = ';'.join(parts) + ';' + ''.join(f'[s{i}]' for i in range(n)) + f'concat=n={n}:v=1:a=0[cat]'
    if gif:
        graph += ';[cat]split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=none[out]'
        run(segment_inputs(segments) + ['-filter_complex', graph, '-map', '[out]', str(out)])
    else:
        graph += ';[cat]format=yuv420p[out]'
        run(segment_inputs(segments) + ['-filter_complex', graph, '-map', '[out]', '-c:v', 'libx264', str(out)])
    return out


def main():
    want = set(sys.argv[1:]) or {'duel-speed', 'trailer', 'highlights'}
    if 'duel-speed' in want:
        print('wrote', duel_speed())
    if 'trailer' in want:
        print('wrote', concat(TRAILER, 720, 480, 30, VIDEOS / 'trailer.mp4', gif=False))
    if 'highlights' in want:
        print('wrote', concat(HIGHLIGHTS, 480, 320, 20, GIFS / 'highlights.gif', gif=True))


if __name__ == '__main__':
    main()
