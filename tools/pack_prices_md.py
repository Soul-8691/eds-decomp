#!/usr/bin/env python3
"""Regenerate the usage-stat password prices in PACKS.md (run after usage/*.json or banlist edits).

  python3 tools/pack_prices_md.py [PACKS.md]
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from packs_md import main  # noqa: E402

if __name__ == '__main__':
    main(['--prices'] + sys.argv[1:])
