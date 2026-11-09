"""Shared .ydk deck list parsing (passcodes and YGOPRODeck name comments)."""

from __future__ import annotations

import re

YDK_SECTIONS = frozenset({'#main', '#extra', '#side', '!side'})
PASSCODE_LINE = re.compile(r'^(\d{6,10})(?:\s+--.*)?$')


def parse_passcode(line: str) -> int | None:
    """The passcode on a .ydk card line, or None if the line is not a passcode entry."""
    line = line.strip()
    if line.isdigit():
        return int(line)
    m = PASSCODE_LINE.match(line)
    return int(m.group(1)) if m else None


def read_ydk(path: str) -> dict[str, list[int]]:
    """Section name -> passcodes (#main, #extra, !side; !side is ignored by callers that do not use it)."""
    sections: dict[str, list[int]] = {s: [] for s in ('#main', '#extra', '!side')}
    sec: str | None = None
    for line in open(path, encoding='utf-8', errors='replace'):
        line = line.strip()
        low = line.lower()
        if low in YDK_SECTIONS:
            sec = low if low != '#side' else '!side'
        elif sec:
            pid = parse_passcode(line)
            if pid is not None:
                sections[sec].append(pid)
    return sections
