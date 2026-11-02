#!/usr/bin/env python3
"""Regenerate the pack pull-rate tables in PACKS.md (run after adding cards or editing packs/*_tcg.json).

  python3 tools/pack_chances_md.py [PACKS.md]
"""
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from packs_md import main  # noqa: E402

if __name__ == '__main__':
    main(['--chances'] + sys.argv[1:])
