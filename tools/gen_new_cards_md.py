#!/usr/bin/env python3
"""Regenerate repo-root NEW_CARDS.md from new_cards.txt (demo list above ------)."""

from pathlib import Path
from urllib.parse import quote


def load_demo_cards(path: Path) -> list[str]:
    names: list[str] = []
    for line in path.read_text(encoding="utf-8").splitlines():
        line = line.strip()
        if line.startswith("------"):
            break
        if not line or line.startswith("#"):
            continue
        names.append(line.split("|", 1)[0].strip())
    return names


def img_cell(name: str) -> str:
    enc = quote(name, safe="")
    return (
        f'<td align="center">'
        f'<img src="cards/{enc}.png" alt="{name}" width="240"></td>'
    )


def main() -> None:
    root = Path(__file__).resolve().parents[1]
    cards = load_demo_cards(root / "new_cards.txt")
    rows: list[str] = []
    for i in range(0, len(cards), 4):
        chunk = cards[i : i + 4]
        cells = "".join(img_cell(n) for n in chunk)
        while len(chunk) < 4:
            cells += '<td></td>'
            chunk.append("")
        rows.append(f"<tr>\n{cells}\n</tr>")

    body = "\n".join(rows)
    n = len(cards)
    md = f"""# New cards

The {n} cards added in the demo card list (the repo-root `new_cards.txt`, above `------`), as the game shows them in Deck Edit's Card View. The screenshots are taken by [`tools/card_screenshots.lua`](tools/card_screenshots.lua) in mGBA.

<table>
{body}
</table>
"""
    (root / "NEW_CARDS.md").write_text(md, encoding="utf-8", newline="\n")
    print(f"NEW_CARDS.md: {n} cards")


if __name__ == "__main__":
    main()
