#!/usr/bin/env python3
"""The README's duel screenshots and recordings (README_TODO.md) as mGBA scripts: tools/media/<name>_Media_Player.lua.

    python3 tools/media/gen_duel_media.py

Each capture is a Test Duel board set up like a TESTING.md scenario (tools/testing_scenarios/exact.py: setup and
steps) played by lib_steps.lua, with the media steps:
    point <place>            the cursor on a card (the info bar shows it)
    extra_summon <Card> from <materials> -> <m0..m4|emz0|emz1> [atk|def]
                             a Synchro, Xyz or Link Summon from the Extra Deck pile
    options speed=default|always|super anims=show|hide battle=show|hide
                             the Settings bits (gSaveData.options), read live by the duel
    hold R|L|B / release R|L|B   a key held across the following steps
    open_menu <place>        the card's menu, left open
    close                    B
    wait <frames>
    shot <name>              screenshots/<name>.png (240x160)
    gif_start <name> / mp4_start <name> ... rec_stop
                             screenshots/gifs/<name>.gif (2x, 30 fps) / screenshots/videos/<name>.mp4 (3x)
Run them with run_tests.ps1 -Media (tools/media/README.md). The test-scenario MP4 capture is off for these.
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'tools'))
sys.path.insert(0, str(ROOT / 'tools/testing_scenarios'))
import exact                                    # noqa: E402
import gen_testing_lua                          # noqa: E402

OUT = ROOT / 'tools/media'

# The video flags (exact.VIDEO_KEYS). A card effect is recorded like the card-test videos: no chain list, card-show
# animations, prompts or battle scenes, and the frames of prompts and lists left out (CLEAN). A capture shows one only
# when that is what it is about.
CLEAN = None
# Synchro, Xyz, Link: the list, the material prompts and the summoned monster's card
SHOW_PICKS = {'prompts': 'show', 'lists': 'show', 'cards': 'show'}

# Captures follow a few rules (the user's): the cards on a board sit next to each other; a monster attacks only a
# weaker one; a recording starts once the cursor rests on the first card used (so it never crosses empty hand slots
# showing the names of the hand the setup replaced); its still tail is trimmed when it stops (lib_steps.lua).


def speed_turn(mode):
    """Duel speeds (README_TODO.md): the same turn at one speed; stitch_media.py puts the three side by side."""
    return ('Duel Speed ' + mode.capitalize(), 'Goat', None,
            '''you.hand: Breaker the Magical Warrior
               opp.m0: Mystical Elf atk
               opp.st0: Call of the Haunted set''',
            '''options speed=%s
               point you.hand[Breaker the Magical Warrior]
               wait 30
               mp4_start duel-speed-%s
               summon you.hand[Breaker the Magical Warrior] -> m0 atk
               activate you.m0
               target opp.st0
               battle
               attack you.m0 -> opp.m0
               wait 30
               rec_stop''' % (mode, mode),
            {'video': {'cards': 'show', 'battle': 'show'}})


# (capture name, the format whose usage deck the CPU plays, start, setup, steps[, extra])
# extra: {'video': the video flags (default: the card tests' clean video), 'actor': 'CPU' (the CPU's turn),
#         'freeze': False (the CPU answers: chains, responses)}
MEDIA = [
    # ---- "In the duel": the screenshots the README has (replaced) ----
    ('Card Icons', 'Yata', None,
     '''you.hand: Gravekeeper's Spy, Breaker the Magical Warrior, Tribe-Infecting Virus, Jinzo, Gemini Elf
        opp.m0: Mechanicalchaser atk''',
     '''point you.hand[Gravekeeper's Spy]
        wait 30
        shot Card Icons'''),
    ('Card Icons Up Close', 'Scientist', None,
     '''you.hand: Gilasaurus, Gemini Elf, Breaker the Magical Warrior, Mechanicalchaser, Sangan
        you.m0: Gemini Elf atk''',
     '''open_menu you.hand[Gilasaurus]
        shot Card Icons (Up Close) - Summon-Set Order
        close'''),
    ('Activate Set Order', 'Yata', None,
     '''you.hand: Raigeki, Fissure, Pot of Greed, Graceful Charity, Book of Moon
        opp.m0: Mechanicalchaser atk''',
     '''open_menu you.hand[Raigeki]
        shot Activate-Set Order
        close'''),
    ('Counters', 'Scientist', None,
     '''you.hand: Breaker the Magical Warrior''',
     '''summon you.hand[Breaker the Magical Warrior] -> m0 atk
        point you.m0
        wait 30
        shot Counters'''),
    # ---- new: the header's duel in progress, the full board, the chain list ----
    ('Duel', 'Goat', {'lp': (5600, 3900)},
     '''you.hand: Graceful Charity, Sangan, Smashing Ground
        you.m0: Breaker the Magical Warrior atk
        you.m1: Jinzo atk
        you.st0: Torrential Tribute set
        you.st1: Call of the Haunted set
        opp.m0: Mechanicalchaser atk
        opp.m1: Gemini Elf set
        opp.st0: Trap Hole set
        opp.hand: Fissure, Gemini Elf''',
     '''point you.m1
        wait 30
        shot Duel'''),
    ('Duel Board', 'Goat', {'lp': (6400, 5100)},
     '''you.m0: Gemini Elf atk
        you.m1: Breaker the Magical Warrior atk
        you.m2: Mechanicalchaser def
        you.st0: Torrential Tribute set
        you.st1: Call of the Haunted set
        opp.m0: Jinzo atk
        opp.m1: Gemini Elf set
        opp.st0: Trap Hole set
        opp.st1: Swords of Revealing Light up''',
     '''point you.m1
        wait 30
        shot Duel Board'''),
    ('Chain List', 'Goat', None,
     '''you.hand: Mystical Space Typhoon
        you.st0: Mystical Space Typhoon set
        opp.st0: Call of the Haunted set
        opp.st1: Trap Hole set''',
     # (a 2-link chain: the Set Mystical Space Typhoon answers the one from the hand; the list, with both, shows from
     # about 75 frames after the second target)
     '''activate you.hand[Mystical Space Typhoon]
        target opp.st0
        when prompt response: activate you.st0
        target opp.st1
        wait 100
        shot Chain List'''),
    # ---- Synchro, Xyz and Link (GIFs) ----
    ('Synchro', 'Goat', None,
     '''you.m0: Junk Synchron atk
        you.m1: Cyber Dragon atk
        you.extra: Stardust Dragon''',
     '''point you.m0
        wait 50
        gif_start synchro
        extra_summon Stardust Dragon from you.m0, you.m1 -> m0 atk
        point you.m0
        settle
        wait 20
        rec_stop''',
     {'video': SHOW_PICKS}),
    ('Xyz', 'Goat', None,
     '''you.m0: Gemini Elf atk
        you.m1: Gemini Elf atk
        you.extra: Number 39: Utopia''',
     '''point you.m0
        wait 50
        gif_start xyz
        extra_summon Number 39: Utopia from you.m0, you.m1 -> m0 atk
        point you.m0
        settle
        wait 20
        rec_stop''',
     {'video': SHOW_PICKS}),
    # (competitive: Sangan and Witch of the Black Forest become Proxy Dragon, and each searches as it goes to the
    # Graveyard; Night Assailant and Spirit Reaper fit both searches, so the order of the two lists does not matter)
    ('Link', 'Goat', None,
     '''you.m0: Sangan atk ss
        you.m1: Witch of the Black Forest atk ss
        you.deck: Night Assailant, Spirit Reaper
        you.extra: Proxy Dragon''',
     '''point you.m0
        wait 50
        gif_start link
        extra_summon Proxy Dragon from you.m0, you.m1 -> emz0
        pick you.deck[Night Assailant]
        pick you.deck[Spirit Reaper]
        settle
        wait 20
        rec_stop''',
     {'video': SHOW_PICKS}),
    ('Zone Pick', 'Goat', None,
     '''you.hand: Gemini Elf
        you.m0: Mechanicalchaser atk''',
     '''point you.hand[Gemini Elf]
        wait 40
        gif_start zone-pick
        summon you.hand[Gemini Elf] -> m1 atk
        wait 30
        rec_stop''',
     {'video': {'prompts': 'show'}}),
    # ---- Fixes to the original game (GIFs) ----
    # (competitive: the CPU's Giant Trunade clears your backrow, a Set Mirror Force among it; the monster your Call of
    # the Haunted revived goes with the Call. Neither Set card can answer, so no question interrupts.)
    ('Call Trunade', 'Goat', None,
     '''you.st0: Call of the Haunted set
        you.st1: Mirror Force set
        you.gy: Summoned Skull
        opp.hand: Giant Trunade''',
     '''activate you.st0
        pick you.gy[Summoned Skull]
        settle
        point you.m0
        wait 30
        gif_start call-trunade
        unfreeze
        pass
        await Giant Trunade
        rec_stop''',
     {'video': CLEAN}),
    ('Graverobber', 'Goat', None,
     '''you.st0: Graverobber set
        opp.gy: Pot of Greed''',
     '''point you.st0
        wait 30
        gif_start graverobber
        activate you.st0
        pick opp.gy[Pot of Greed]
        wait 120
        set you.hand[Pot of Greed] -> st1
        menu set
        wait 60
        activate you.st[Pot of Greed]
        settle
        wait 20
        rec_stop''',
     {'video': {'lists': 'show'}}),
    # ---- The new cards (video) ----
    ('New Cards', 'Goat', None,
     '''you.hand: Cyber Dragon, Breaker the Magical Warrior, Polymerization, Relinquished, Thousand-Eyes Idol
        you.extra: Thousand-Eyes Restrict
        opp.m0: Summoned Skull atk
        opp.m1: Mechanicalchaser atk
        opp.st0: Call of the Haunted set''',
     '''point you.hand[Cyber Dragon]
        wait 30
        mp4_start new-cards
        special_summon you.hand[Cyber Dragon] -> m0 atk
        wait 40
        summon you.hand[Breaker the Magical Warrior] -> m1 atk
        wait 40
        activate you.m1
        target opp.st0
        wait 40
        fusion_summon Thousand-Eyes Restrict from you.hand[Relinquished], you.hand[Thousand-Eyes Idol] -> m2 atk
        wait 40
        activate you.m2
        target opp.m0
        wait 40
        point you.m2
        settle
        wait 20
        rec_stop''',
     {'video': CLEAN}),
    # ---- Smarter CPU (video): Magic Cylinder on Black Luster Soldier's direct attack ----
    ('CPU Play', 'Goat', None,
     '''you.m0: Black Luster Soldier - Envoy of Beginning atk
        opp.st0: Magic Cylinder set
        opp.hand: Sangan''',
     '''point you.m0
        wait 30
        mp4_start cpu-play
        battle
        attack you.m0 -> direct
        settle
        wait 20
        rec_stop''',
     {'video': CLEAN, 'freeze': False}),
    # ---- Settings (videos and a GIF) ----
    speed_turn('default'),
    speed_turn('always'),
    speed_turn('super'),
    ('Hide Anims', 'Goat', None,
     '''you.hand: Pot of Greed, Pot of Greed''',
     '''options anims=show
        point you.hand[0]
        wait 30
        mp4_start hide-anims
        activate you.hand[Pot of Greed]
        wait 40
        options anims=hide
        activate you.hand[Pot of Greed]
        settle
        wait 20
        rec_stop''',
     {'video': {'cards': 'show'}}),
    ('Hide Battle', 'Goat', None,
     '''you.m0: Summoned Skull atk
        opp.m0: Mechanicalchaser atk''',
     '''options battle=hide
        battle
        point you.m0
        wait 30
        gif_start hide-battle
        attack you.m0 -> opp.m0
        settle
        wait 20
        rec_stop''',
     {'video': {'battle': 'show'}}),
    # ---- Controls (GIFs): R held through the CPU's chain; L held through its turn (debug builds) ----
    # Hold R: your Set Mystical Space Typhoon (named in the info bar first) could answer the CPU's Pot of Greed; with R
    # held no prompt comes.
    ('Hold R', 'Goat', None,
     '''you.st0: Mystical Space Typhoon set
        opp.hand: Pot of Greed''',
     '''point you.st0
        wait 30
        gif_start hold-r
        wait 45
        pass
        hold R
        await Pot of Greed
        rec_stop
        release R
        cpu_turn''',
     {'video': {'chain': 'show', 'prompts': 'show'}, 'freeze': False}),
    ('Hold L', 'Goat', None,
     '''you.m0: Gemini Elf atk''',
     '''point you.m0
        wait 30
        gif_start hold-l
        pass
        hold L
        cpu_turn
        release L
        rec_stop''',
     {'video': CLEAN, 'freeze': False}),
]


def sc_of(name, start, setup, steps, extra=None):
    extra = extra or {}
    st = dict(exact.DEFAULT_START)
    st.update(start or {})
    return {'actor': extra.get('actor', 'Player'), 'effect': 'media', 'start': st, 'setup': exact._lines(setup),
            'steps': exact._lines(steps), 'expect': [], 'note': None,
            'video': extra['video'] if 'video' in extra else {'effect': 'none', 'chain': 'show'}}


def main():
    written = []
    for item in MEDIA:
        name, fmt, start, setup, steps = item[:5]
        extra = item[5] if len(item) > 5 else {}
        sc = sc_of(name, start, setup, steps, extra)
        entry = {'card': name, 'format': fmt, 'sc': sc, 'basename': exact.basename(name, sc)}
        fields = gen_testing_lua.scenario_fields(entry)
        if 'freeze' in extra:
            fields = [('    freezeOpponent = %s,' % ('true' if extra['freeze'] else 'false'))
                      if f.startswith('    freezeOpponent =') else f for f in fields]
        body = '\n'.join([
            '-- AUTO-GENERATED by tools/media/gen_duel_media.py: a README duel capture (README_TODO.md)',
            gen_testing_lua.LUA_BOOTSTRAP,
            'EDS.run({',
            *fields,
            '    captureMp4 = false,',
            '})',
        ]) + '\n'
        path = OUT / (entry['basename'] + '.lua')
        path.write_text(body, encoding='utf-8')
        written.append(path.name)
    print('Wrote %d scripts: %s' % (len(written), ', '.join(written)))


if __name__ == '__main__':
    main()
