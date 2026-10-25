#!/usr/bin/env python3
"""Compare goat-cards-fusion.txt against include/constants/cards.h (gencards ident keys)."""
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def ident(name: str) -> str:
    s = name.upper().replace("&", " AND ").replace("'", "").replace(".", " ").replace("#", " ")
    s = s.replace("\u03b1", "A").replace("\u0391", "A")
    s = re.sub(r"[^A-Z0-9]+", "_", s).strip("_")
    return "CARD_" + s


def symbol_to_key(sym: str) -> str:
    if sym.startswith("CARD_"):
        sym = sym[5:]
    sym = re.sub(r"_ALT\d*$", "", sym)
    return sym


def parse_header_keys(path):
    keys = set()
    pattern = re.compile(r"^\s*(CARD_[A-Z0-9_]+)\s*=")
    for line in path.read_text(encoding="utf-8").splitlines():
        m = pattern.match(line)
        if not m:
            continue
        sym = m.group(1)
        if re.match(r"^CARD_\d+$", sym):
            continue
        keys.add(symbol_to_key(sym))
    return keys


def main() -> None:
    header_keys = parse_header_keys(ROOT / "include/constants/cards.h")
    goat_path = ROOT / "goat-cards-fusion.txt"
    goat = [ln.strip() for ln in goat_path.read_text(encoding="utf-8").splitlines() if ln.strip()]
    missing = []
    for name in goat:
        if symbol_to_key(ident(name)) not in header_keys:
            missing.append(name)
    out = ROOT / "missing-fusion.txt"
    out.write_text("\n".join(missing) + ("\n" if missing else ""), encoding="utf-8")
    print(f"header keys: {len(header_keys)}")
    print(f"goat fusion lines: {len(goat)}")
    print(f"in EDS: {len(goat) - len(missing)}")
    print(f"missing -> {out.name}: {len(missing)}")


if __name__ == "__main__":
    main()
