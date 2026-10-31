#!/usr/bin/env python3
"""Build packs/Dark Revelation Volume 1_tcg.json from Yugipedia's English card list table."""
import json
import re
import sys
import urllib.request

URL = 'https://yugipedia.com/wiki/Dark_Revelation_Volume_1'
OUT = 'packs/Dark Revelation Volume 1_tcg.json'


def parse_name(raw):
    raw = raw.strip()
    m = re.search(r'\(as "([^"]+)"\)', raw)
    if m:
        return m.group(1).strip()
    m = re.search(r'"([^"]+)"', raw)
    if m:
        return m.group(1).strip()
    return raw.strip().strip('"')


def ingest(text):
    # Markdown / wikitext rows: | DR1-EN001 | " Name" | Rarity | Category |
    pat = re.compile(
        r'\|\s*DR1-EN(\d{3})\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*([^|]+?)\s*\|'
    )
    out = {}
    for num, name_cell, rarity, _cat in pat.findall(text):
        if int(num) > 267:
            continue
        rarity = rarity.strip()
        if not rarity or 'tip card' in rarity.lower():
            continue
        name = parse_name(name_cell)
        if not name:
            continue
        out[name] = [rarity]
    return out


def main():
    if len(sys.argv) > 1:
        text = open(sys.argv[1], encoding='utf-8').read()
    else:
        text = urllib.request.urlopen(URL, timeout=60).read().decode('utf-8', 'replace')
    out = ingest(text)
    with open(OUT, 'w', encoding='utf-8') as f:
        json.dump(out, f, indent=2, ensure_ascii=False)
        f.write('\n')
    print(f'wrote {OUT} ({len(out)} cards)')


if __name__ == '__main__':
    main()
