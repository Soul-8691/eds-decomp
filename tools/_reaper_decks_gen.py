#!/usr/bin/env python3
import deckgen

DECKS = [
    ("Reaper_Stein_Monarch", "decks/Jazz-Stein Monarch.ydk", "Jazz-Stein Monarch.ydk"),
    ("Reaper_Warrior", "decks/tobitomatenkopf-Warrior.ydk", "tobitomatenkopf-Warrior.ydk"),
    ("Reaper_Bazoo_Return", "decks/brahimpoke-Bazoo Return.ydk", "brahimpoke-Bazoo Return.ydk"),
    ("Reaper_Chaos_Control", "decks/jcvd93-Chaos Control.ydk", "jcvd93-Chaos Control.ydk"),
    ("Reaper_Tomato_Control", "decks/Roland Gress-Tomato Control.ydk", "Roland Gress-Tomato Control.ydk"),
]

if __name__ == "__main__":
    for t in DECKS:
        deckgen.emit_deck(*t)
