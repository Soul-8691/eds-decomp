#!/usr/bin/env python3
"""The README's stitched media (README_TODO.md), made with ffmpeg from the captures in screenshots/:

    python3 tools/media/stitch_media.py [duel-speed] [trailer] [highlights] [cpu-play]   (all when none is named)

  screenshots/gifs/cpu-play.gif       "The CPU at work": the ai-*.mp4 clips (gen_duel_media.py, one AI behavior each),
                                      at normal speed

  screenshots/gifs/duel-speed.gif     the same, as a GIF (240x160 each), for README_WIP.md

  screenshots/videos/duel-speed.mp4   the same turn at DEFAULT, ALWAYS and SUPER side by side (each labelled; the
                                      shorter ones hold their last frame)
  screenshots/videos/trailer.mp4      about 45 s: the title screen, the duelist select, a pre-duel screen, rock-paper-
                                      scissors, a Synchro Summon, a Link Summon, the CPU's Magic Cylinder, a pack
  screenshots/gifs/highlights.gif     README_WIP.md's TLDR: about 120 s of highlighted card-test videos
                                      (tools/testing_scenarios/recordings/keep/highlights/) at 2x speed: about 60 s

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


def duel_speed_gif():
    """screenshots/gifs/duel-speed.gif (README_WIP.md): the three speeds side by side, each 240x160 and labelled, at
    20 fps; the shorter ones hold their last frame."""
    modes = ['default', 'always', 'super']
    inputs, parts = [], []
    for i, m in enumerate(modes):
        inputs += ['-i', str(VIDEOS / f'duel-speed-{m}.mp4')]
        parts.append(f"[{i}:v]scale=240:160:flags=neighbor,fps=20,drawtext=fontfile='{FONT}':text='{m.upper()}':"
                     f"x=4:y=4:fontsize=12:fontcolor=white:borderw=2:bordercolor=black[v{i}]")
    graph = (';'.join(parts) + ';[v0][v1][v2]hstack=inputs=3,split[a][b];[a]palettegen=stats_mode=diff[p];'
             '[b][p]paletteuse=dither=none[out]')
    out = GIFS / 'duel-speed.gif'
    run(inputs + ['-filter_complex', graph, '-map', '[out]', str(out)])
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

# The TLDR's card effects (2026-11-17): whole highlighted card-test videos, about 120 s in all, played at 2x
TESTS = ROOT / 'tools/testing_scenarios/recordings/keep/highlights'
HIGHLIGHT_SPEED = 2.0
# (in the order the cards were first released in the TCG: their first set in packs/*_tcg.json, with
# format_packs.TCG_RELEASE_DATE; one set's cards by name)
HIGHLIGHTS = [(TESTS / f'{name}.mp4', None, None) for name in (
    'Card_Destruction_Discard_And_Draw_Player',
    'Soul_Exchange_Cpu_Tribute_Opponent_Monster_CPU',
    'Limiter_Removal_Double_Machine_Atk_Player',
    'Thousand_Eyes_Restrict_Absorb_And_Attack_Player',
    'Kycoo_the_Ghost_Destroyer_Cpu_Banish_On_Battle_Damage_CPU',
    'Torrential_Tribute_Cpu_On_Human_Summon_CPU',
    'Airknight_Parshath_Cpu_Pierce_And_Draw_CPU',
    'Dark_Ruler_Ha_Des_Cpu_Negate_Sangan_CPU',
    'Exiled_Force_Cpu_Tribute_Destroy_Stronger_CPU',
    'Injection_Fairy_Lily_Pay_2000_For_3000_Player',
    'Gravekeepers_Spy_Cpu_Flipped_By_Attack_CPU',
    'Newdoria_Cpu_Destroy_Strongest_CPU',
    'Trap_Dustshoot_Return_Hand_Monster_Player',
    'Breaker_the_Magical_Warrior_Counter_Then_Destroy_Spell_Trap_Player',
    'DD_Warrior_Lady_Banish_Both_Player',
    'Chaos_Emperor_Dragon_Envoy_of_the_End_Summon_And_Wipe_Player',
    'Enemy_Controller_Cpu_Take_Stronger_Monster_CPU',
    'Mystik_Wok_Cpu_Low_Lp_Answer_Attack_CPU',
    'Brain_Control_Cpu_Pay_800_Take_Control_CPU',
    'Cyber_End_Dragon_Cpu_Pierce_CPU',
)]


# The CPU at work (2026-11-17): the AI behavior clips of gen_duel_media.py, at normal speed
CPU_PLAY_SPEED = 1.0
# (name, or (name, length in s) to cut a clip's tail: the lethal clips end on YOU LOSE, before the next screen)
CPU_PLAY = [(VIDEOS / f'ai-{n}.mp4', None, None) if isinstance(n, str) else (VIDEOS / f'ai-{n[0]}.mp4', None, n[1])
            for n in (
    'draw-first', 'thunder-dragon', 'dark-hole', 'nobleman', 'exiled-force', 'virus', 'enemy-controller',
    'steal-tribute', 'tribute-upgrade', 'hane-hane', 'face-down-attack', 'kycoo', 'chaos-emperor', 'recruiter',
    'man-eater-bug', 'waboku', 'trap-hole', 'solemn', 'magic-cylinder', ('snatch-lethal', 14.5),
)]


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


def concat(segments, width, height, fps, out, gif, speed=1.0):
    n = len(segments)
    pts = f'setpts=PTS/{speed},' if speed != 1.0 else ''
    parts = [f'[{i}:v]{pts}scale={width}:{height}:flags=neighbor,fps={fps},setsar=1,format=rgb24[s{i}]'
             for i in range(n)]
    graph = ';'.join(parts) + ';' + ''.join(f'[s{i}]' for i in range(n)) + f'concat=n={n}:v=1:a=0[cat]'
    if gif:
        graph += ';[cat]split[a][b];[a]palettegen=stats_mode=diff[p];[b][p]paletteuse=dither=none[out]'
        run(segment_inputs(segments) + ['-filter_complex', graph, '-map', '[out]', str(out)])
    else:
        graph += ';[cat]format=yuv420p[out]'
        run(segment_inputs(segments) + ['-filter_complex', graph, '-map', '[out]', '-c:v', 'libx264', str(out)])
    return out


def main():
    want = set(sys.argv[1:]) or {'duel-speed', 'trailer', 'highlights', 'cpu-play'}
    if 'duel-speed' in want:
        print('wrote', duel_speed())
        print('wrote', duel_speed_gif())
    if 'trailer' in want:
        print('wrote', concat(TRAILER, 720, 480, 30, VIDEOS / 'trailer.mp4', gif=False))
    if 'highlights' in want:
        print('wrote', concat(HIGHLIGHTS, 240, 160, 20, GIFS / 'highlights.gif', gif=True, speed=HIGHLIGHT_SPEED))
    if 'cpu-play' in want:
        print('wrote', concat(CPU_PLAY, 240, 160, 20, GIFS / 'cpu-play.gif', gif=True, speed=CPU_PLAY_SPEED))


if __name__ == '__main__':
    main()
