#!/usr/bin/env python3
"""Append YGOPRODeck card names to passcode lines in .ydk deck files.

  python3 tools/annotate_ydk.py [decks/*.ydk ...]   # default: decks/

Each passcode line becomes ``<id> -- <name>``. Section headers and other lines are unchanged.
Names come from https://db.ygoprodeck.com/api/v7/cardinfo.php (id query, batched).
"""

from __future__ import annotations

import json
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from pathlib import Path

API = 'https://db.ygoprodeck.com/api/v7/cardinfo.php?'
BATCH = 50
PASSCODE_RE = re.compile(r'^(\d{6,10})\s*(?:--\s*(.*))?$')
SECTION_MARKERS = frozenset({'#main', '#extra', '#side', '!side'})


def http_get(url: str) -> bytes:
    req = urllib.request.Request(url, headers={'User-Agent': 'eds-decomp tools/annotate_ydk.py'})
    with urllib.request.urlopen(req, timeout=120) as r:
        return r.read()


def fetch_names(ids: list[int]) -> dict[int, str]:
    """{passcode: name} from YGOPRODeck."""
    out: dict[int, str] = {}
    for i in range(0, len(ids), BATCH):
        chunk = ids[i : i + BATCH]
        q = urllib.parse.urlencode({'id': ','.join(str(x) for x in chunk)})
        try:
            data = json.loads(http_get(API + q))['data']
        except urllib.error.HTTPError as e:
            if e.code != 400:
                raise
            for pid in chunk:
                single = json.loads(http_get(API + urllib.parse.urlencode({'id': pid})))['data']
                for c in single:
                    out[c['id']] = c['name']
                time.sleep(0.06)
            continue
        for c in data:
            out[c['id']] = c['name']
        time.sleep(0.06)
    return out


def collect_ids(paths: list[Path]) -> list[int]:
    seen: set[int] = set()
    for path in paths:
        for line in path.read_text(encoding='utf-8', errors='replace').splitlines():
            m = PASSCODE_RE.match(line.strip())
            if m:
                seen.add(int(m.group(1)))
    return sorted(seen)


def annotate_file(path: Path, names: dict[int, str]) -> str:
    lines_out: list[str] = []
    missing: list[int] = []
    for raw in path.read_text(encoding='utf-8', errors='replace').splitlines():
        line = raw.rstrip('\r')
        stripped = line.strip()
        low = stripped.lower()
        if not stripped or low in SECTION_MARKERS or stripped.startswith('#'):
            lines_out.append(line)
            continue
        m = PASSCODE_RE.match(stripped)
        if not m:
            lines_out.append(line)
            continue
        pid_str, pid = m.group(1), int(m.group(1))
        name = names.get(pid)
        if name is None:
            missing.append(pid)
            lines_out.append(line)
        else:
            lines_out.append(f'{pid_str} -- {name}')
    if missing:
        uniq = sorted(set(missing))
        print(f'annotate_ydk: {path}: no YGOPRODeck name for {len(uniq)} id(s): '
              f'{", ".join(str(x) for x in uniq[:8])}' + ('…' if len(uniq) > 8 else ''), file=sys.stderr)
    return '\n'.join(lines_out) + '\n'


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    if len(sys.argv) > 1:
        paths = [Path(p) for p in sys.argv[1:]]
    else:
        paths = sorted((root / 'decks').glob('*.ydk'))
    if not paths:
        sys.exit('annotate_ydk: no .ydk files')
    ids = collect_ids(paths)
    print(f'annotate_ydk: {len(ids)} unique passcodes in {len(paths)} file(s)', file=sys.stderr)
    names = fetch_names(ids)
    # Konami vs YGOPRODeck id quirks used elsewhere in the repo (tools/testdeck.py).
    names.setdefault(11901678, 'B. Skull Dragon')
    names.setdefault(98502113, 'Dark Paladin')
    names.setdefault(70095154, 'Cyber Dragon')
    names.setdefault(1546123, 'Cyber End Dragon')
    names.setdefault(23995346, 'Manticore of Darkness')
    names.setdefault(10248389, 'Cyber Blader')
    names.setdefault(86099788, 'The Last Warrior from Another Planet')
    for path in paths:
        text = annotate_file(path, names)
        if path.read_text(encoding='utf-8', errors='replace') != text:
            path.write_text(text, encoding='utf-8', newline='\n')
            print(f'annotate_ydk: wrote {path}', file=sys.stderr)


if __name__ == '__main__':
    main()
