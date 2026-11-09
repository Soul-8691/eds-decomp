#!/usr/bin/env python3
"""Emit one mGBA Lua script per TESTING.md scenario.

Recording file basename: {card name}_{effect name}_{Player|CPU}.mp4 under tools/testing_scenarios/recordings/

  python3 tools/gen_testing_lua.py
  python3 tools/gen_testing_lua.py --poc-only
  python3 tools/gen_testing_lua.py --list-only
"""
from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TESTING = ROOT / 'TESTING.md'
OUT_DIR = ROOT / 'tools' / 'testing_scenarios' / 'generated'
POC_DIR = ROOT / 'tools' / 'testing_scenarios' / 'poc'
MANIFEST = ROOT / 'tools' / 'testing_scenarios' / 'manifest.json'

POC_CARDS = {
    'Card Destruction',
    'Soul Exchange',
    'Goblin Attack Force',
    'Torrential Tribute',
    'Exiled Force',
}

# Stable effect slugs for POC cards (must match keys in poc_plays.lua).
POC_EFFECT_OVERRIDES: dict[tuple[str, int, str], str] = {
    ('Card Destruction', 1, 'Player'): 'discard_and_draw',
    ('Soul Exchange', 1, 'Player'): 'tribute_opponent_monster',
    ('Goblin Attack Force', 1, 'Player'): 'usage_deck_verify',
    ('Goblin Attack Force', 2, 'Player'): 'mandatory_follow_up',
    ('Torrential Tribute', 1, 'Player'): 'destroy_all_on_summon',
    ('Exiled Force', 1, 'Player'): 'tribute_destroy_target',
}

SCENARIO_LINE = re.compile(r'^\s+(\d+)\.\s+(.*)$')

LUA_BOOTSTRAP = r'''local __edsScriptSrc = debug.getinfo(1, "S").source or ""
local function __edsTestingRoot()
    local src = __edsScriptSrc
    if src:sub(1, 1) == "@" then
        src = src:sub(2)
    end
    src = src:gsub("\\", "/")
    local dir = src:match("^(.*)/[^/]+$")
    if dir and (dir:match("/poc$") or dir:match("/generated$")) then
        return dir .. "/.."
    end
    if dir and dir:match("/testing_scenarios$") then
        return dir
    end
    return "tools/testing_scenarios"
end
local paths = dofile(__edsTestingRoot() .. "/eds_paths.lua")
local EDS = paths.loadEds()'''


def slug(s: str) -> str:
    s = re.sub(r'[^\w\s-]', '', s)
    s = re.sub(r'[-\s]+', '_', s.strip())
    return (s[:56] if s else 'effect').strip('_')


def effect_slug(s: str) -> str:
    """Title-case each underscore segment for recording / script file names."""
    base = slug(s).lower()
    parts = [p.capitalize() for p in base.split('_') if p]
    out = '_'.join(parts) if parts else 'Effect'
    return out[:56]


def human_effect_name(card: str, text: str, coded: str, index: int, total: int) -> str:
    low = text.lower()
    if total > 1:
        if 'mandatory' in low or 'not an optional' in low:
            return 'mandatory_follow_up'
        if text.startswith('Same ') or 'same duel' in low:
            return 'chain_rules'
        if 'per **coded behavior**' in low:
            return 'chain_rules'
    quoted = re.search(r'"([^"]{8,90})"', coded)
    if quoted:
        short = slug(quoted.group(1).split('.')[0])
        if len(short) <= 48:
            return short
    m = re.search(rf'Activate \*\*{re.escape(card)}\*\*', text, re.I)
    if m:
        rest = text[m.end():].lstrip(' :,')
        rest = re.sub(r'\*\*[^*]+\*\*', '', rest).strip()
        if rest:
            return slug(rest.split('.')[0][:48])
    if 'Normal Spell' in coded or 'Normal Trap' in coded:
        tail = coded.split(':', 1)[-1].strip()
        if tail.startswith('"'):
            return slug(tail.strip('"').split('.')[0])
    return f'scenario_{index:02d}'


def ai_effect_name(text: str, index: int) -> str:
    low = text.lower()
    if 'mandatory' in low or 'passive' in low or 'ordinary lines' in low:
        return 'cpu_standard_lines'
    if index > 1:
        return f'cpu_pass_{index:02d}'
    return 'cpu_duel'


def parse_testing_md(text: str) -> list[dict]:
    cards: list[dict] = []
    current_format: str | None = None
    current: dict | None = None
    section: str | None = None

    for line in text.splitlines():
        if line.startswith('## ') and not line.startswith('### '):
            current_format = line[3:].strip()
        elif line.startswith('### '):
            if current is not None:
                cards.append(current)
            current = {
                'format': current_format or '',
                'name': line[4:].strip(),
                'coded': '',
                'scenarios': [],
                'ai_scenarios': [],
            }
            section = None
        elif current is None:
            continue
        elif line.startswith('- **Coded behavior:**'):
            section = 'coded'
            current['coded'] = line.split(':', 1)[1].strip()
        elif line.startswith('- **Scenarios:**'):
            section = 'scenarios'
        elif line.startswith('- **AI scenarios:**'):
            section = 'ai_scenarios'
        elif line.startswith('- **') and section != 'coded':
            section = None
        elif section == 'scenarios':
            m = SCENARIO_LINE.match(line)
            if m:
                current['scenarios'].append(m.group(2))
        elif section == 'ai_scenarios':
            m = SCENARIO_LINE.match(line)
            if m:
                current['ai_scenarios'].append(m.group(2))
        elif section == 'coded' and line.startswith('  '):
            current['coded'] += ' ' + line.strip()

    if current is not None:
        cards.append(current)
    return cards


def scenario_entries(card: dict) -> list[dict]:
    out: list[dict] = []
    n = len(card['scenarios'])
    for i, text in enumerate(card['scenarios'], start=1):
        effect = POC_EFFECT_OVERRIDES.get((card['name'], i, 'Player')) or human_effect_name(
            card['name'], text, card['coded'], i, n
        )
        out.append({
            'card': card['name'],
            'format': card['format'],
            'actor': 'Player',
            'effect': effect,
            'text': text,
            'basename': f"{slug(card['name'])}_{effect_slug(effect)}_Player",
        })
    for i, text in enumerate(card['ai_scenarios'], start=1):
        if text.strip().lower().startswith('second pass'):
            continue
        effect = ai_effect_name(text, i)
        out.append({
            'card': card['name'],
            'format': card['format'],
            'actor': 'CPU',
            'effect': effect,
            'text': text,
            'basename': f"{slug(card['name'])}_{effect_slug(effect)}_CPU",
        })
    return out


def lua_body(entry: dict) -> str:
    text_esc = entry['text'].replace('\\', '\\\\').replace('"', '\\"')
    record_frames = '3600' if entry['actor'] == 'CPU' else 'nil'
    play = '{ { wait = 90 } }' if entry['actor'] == 'Player' else 'nil'
    lines = [
        "-- AUTO-GENERATED from TESTING.md",
        f"-- Recording: {entry['basename']}.mp4",
        LUA_BOOTSTRAP,
        "EDS.run({",
        f"    card = {json.dumps(entry['card'])},",
        f"    effect = {json.dumps(entry['effect'])},",
        f"    actor = {json.dumps(entry['actor'])},",
        f"    format = {json.dumps(entry['format'])},",
        f'    scenarioText = "{text_esc}",',
        f"    play = {play},",
        f"    recordFrames = {record_frames},",
        "})",
    ]
    return '\n'.join(lines) + '\n'


def fix_poc_lua_body(entry: dict) -> str:
    """POC scripts with cleaner CPU handling."""
    text_esc = entry['text'].replace('\\', '\\\\').replace('"', '\\"')
    lines = [
        f"-- POC — TESTING.md scenario",
        f"-- Recording: {entry['basename']}.mp4",
        LUA_BOOTSTRAP,
        "local POC = paths.loadPocPlays()",
        f"local key = {json.dumps(entry['basename'])}",
        "local poc = POC[key]",
        "local opts = {",
        f"    card = {json.dumps(entry['card'])},",
        f"    effect = {json.dumps(entry['effect'])},",
        f"    actor = {json.dumps(entry['actor'])},",
        f"    format = {json.dumps(entry['format'])},",
        f'    scenarioText = "{text_esc}",',
        "    setupCards = poc and poc.setupCards,",
        "    play = poc and poc.play,",
        f"    pocRequired = {'true' if entry['actor'] != 'CPU' and poc_key_has_play(entry) else 'false'},",
        "}",
    ]
    if entry['actor'] == 'CPU':
        lines.append("opts.recordFrames = 3600")
        lines.append("opts.play = nil")
        lines.append("opts.freezeOpponent = false")
    elif not poc_key_has_play(entry):
        lines.append("opts.play = { { wait = 120 }, { key = EDS.KEY_START, gap = 30 } }")
    lines.append("EDS.run(opts)")
    return '\n'.join(lines) + '\n'


def poc_key_has_play(entry: dict) -> bool:
    return entry['actor'] == 'Player' and entry['card'] in POC_CARDS


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('--poc-only', action='store_true', help='Only write POC cards (5 cards, all scenarios)')
    ap.add_argument('--list-only', action='store_true')
    args = ap.parse_args()

    text = TESTING.read_text(encoding='utf-8')
    cards = parse_testing_md(text)
    all_entries: list[dict] = []
    for card in cards:
        all_entries.extend(scenario_entries(card))

    if args.list_only:
        print(f"{len(cards)} cards, {len(all_entries)} scenarios")
        for e in all_entries[:5]:
            print(f"  {e['basename']}.mp4")
        print('  ...')
        return 0

    entries = all_entries
    if args.poc_only:
        entries = [e for e in all_entries if e['card'] in POC_CARDS]

    OUT_DIR.mkdir(parents=True, exist_ok=True)
    POC_DIR.mkdir(parents=True, exist_ok=True)
    manifest = []

    for entry in entries:
        fname = entry['basename'] + '.lua'
        poc = entry['card'] in POC_CARDS
        if poc:
            body = fix_poc_lua_body(entry)
            (POC_DIR / fname).write_text(body, encoding='utf-8')
            (OUT_DIR / fname).write_text(lua_body(entry), encoding='utf-8')
        else:
            (OUT_DIR / fname).write_text(lua_body(entry), encoding='utf-8')
        manifest.append({
            'card': entry['card'],
            'effect': entry['effect'],
            'actor': entry['actor'],
            'format': entry['format'],
            'recording': f"tools/testing_scenarios/recordings/{entry['basename']}.mp4",
            'script': (
                f"tools/testing_scenarios/poc/{fname}"
                if poc else f"tools/testing_scenarios/generated/{fname}"
            ),
        })

    MANIFEST.write_text(json.dumps(manifest, indent=2) + '\n', encoding='utf-8')
    print(f"Wrote {len(manifest)} scripts ({sum(1 for e in entries if e['card'] in POC_CARDS)} under poc/)")
    subprocess.run([sys.executable, str(ROOT / 'tools' / 'gen_format_decks_lua.py')], check=False)
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
