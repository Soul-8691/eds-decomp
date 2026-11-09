#!/usr/bin/env python3
"""Run Player POC mGBA scripts and summarize tools/testing_scenarios/recordings/_last_run.log."""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
POC_DIR = ROOT / 'tools' / 'testing_scenarios' / 'poc'
MANIFEST = ROOT / 'tools' / 'testing_scenarios' / 'manifest.json'
RUN_LOG = ROOT / 'tools' / 'testing_scenarios' / 'recordings' / '_last_run.log'
ROM = ROOT / 'eds_demo.gba'

DEFAULT_MGBA = Path(r'C:\Program Files\mGBA\mGBA.exe')

POC_PLAYER_SCRIPTS = [
    'Card_Destruction_Discard_And_Draw_Player.lua',
    'Soul_Exchange_Tribute_Opponent_Monster_Player.lua',
    'Torrential_Tribute_Destroy_All_On_Summon_Player.lua',
    'Exiled_Force_Tribute_Destroy_Target_Player.lua',
    'Goblin_Attack_Force_Usage_Deck_Verify_Player.lua',
]


def find_mgba(explicit: str | None) -> Path:
    if explicit:
        return Path(explicit)
    env = os.environ.get('MGBA_EXE')
    if env and Path(env).is_file():
        return Path(env)
    if DEFAULT_MGBA.is_file():
        return DEFAULT_MGBA
    raise SystemExit('mGBA.exe not found; set MGBA_EXE or install mGBA')


def static_check() -> list[str]:
    errors: list[str] = []
    for name in POC_PLAYER_SCRIPTS:
        path = POC_DIR / name
        if not path.is_file():
            errors.append(f'missing script {path}')
    if MANIFEST.is_file():
        data = json.loads(MANIFEST.read_text(encoding='utf-8'))
        wanted = {f'tools/testing_scenarios/poc/{n}' for n in POC_PLAYER_SCRIPTS}
        have = {e['script'] for e in data if e.get('actor') == 'Player' and 'poc/' in e.get('script', '')}
        if not wanted <= have:
            errors.append(f'manifest missing POC player entries: {wanted - have}')
    return errors


def run_one(mgba: Path, script: Path, timeout_s: int) -> dict:
    RUN_LOG.parent.mkdir(parents=True, exist_ok=True)
    if RUN_LOG.exists():
        RUN_LOG.unlink()
    cmd = [
        str(mgba),
        '-6',
        '-C', 'audioSync=0',
        '-C', 'videoSync=0',
        '-C', 'fpsTarget=6000',
        '-C', 'scaleMultiplier=6',
        '--script', str(script),
        str(ROM),
    ]
    proc = subprocess.Popen(cmd, cwd=str(ROOT))
    deadline = time.time() + timeout_s
    status = 'runner_timeout'
    while time.time() < deadline:
        if proc.poll() is not None:
            status = 'exited'
            break
        if RUN_LOG.exists():
            text = RUN_LOG.read_text(encoding='utf-8', errors='replace')
            if 'PASS ' in text or ('FAIL ' in text and 'missing_poc' in text):
                status = 'logged'
                break
            if text.strip().endswith('play_done=true') or 'PASS ' in text.splitlines()[-1:]:
                status = 'logged'
                break
            last = text.strip().splitlines()[-1] if text.strip() else ''
            if last.startswith('PASS ') or last.startswith('FAIL ') or last.startswith('DONE '):
                status = 'logged'
                break
        time.sleep(0.5)
    if proc.poll() is None:
        proc.terminate()
        try:
            proc.wait(5)
        except subprocess.TimeoutExpired:
            proc.kill()
    log_text = RUN_LOG.read_text(encoding='utf-8', errors='replace') if RUN_LOG.exists() else ''
    return {'script': script.name, 'runner_status': status, 'log': log_text.strip()}


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('--mgba', help='path to mGBA.exe')
    ap.add_argument('--timeout', type=int, default=120, help='seconds per script')
    ap.add_argument('--static-only', action='store_true')
    args = ap.parse_args()

    errors = static_check()
    if errors:
        for e in errors:
            print(e, file=sys.stderr)
        return 1
    print('Static checks: OK (%d POC player scripts)' % len(POC_PLAYER_SCRIPTS))
    if args.static_only:
        return 0

    if not ROM.is_file():
        print(f'Missing ROM {ROM}', file=sys.stderr)
        return 1

    mgba = find_mgba(args.mgba)
    print(f'Using {mgba}')
    results = []
    failed = 0
    for name in POC_PLAYER_SCRIPTS:
        print(f'--- {name} ---')
        r = run_one(mgba, POC_DIR / name, args.timeout)
        results.append(r)
        for line in r['log'].splitlines():
            print('  ', line)
        if 'PASS ' not in r['log']:
            failed += 1
            print('  => not PASS', file=sys.stderr)

    print()
    print(f'Summary: {len(results) - failed}/{len(results)} PASS')
    return 1 if failed else 0


if __name__ == '__main__':
    raise SystemExit(main())
