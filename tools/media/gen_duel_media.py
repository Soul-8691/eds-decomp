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

# The CPU at work: recorded like the card tests, expedited (no chain list, card-showing animations, prompts, lists'
# frames or battle scenes) and with the duel speed at ALWAYS and both Hide settings on (the user's, 2026-11-17).
AI_SHOW = CLEAN


def ai_turn(name, clip, setup, fmt='Goat'):
    """A CPU-turn clip: the set board, then the CPU's whole turn (its draw set by opp.deck); `fmt` is the format
    whose usage deck the rest of the CPU's Deck comes from."""
    return [(name, fmt, None, setup,
             '''point you.m0
                wait 20
                options speed=always anims=hide battle=hide
                mp4_start %s
                pass
                cpu_turn
                rec_stop''' % clip,
             {'video': AI_SHOW, 'freeze': False})]


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
    # The header's duel in progress (GIF), Reaper staples only (usage/reaper.json): Breaker clears the Set Torrential
    # Tribute, then Cyber Dragon (2100) runs over Kycoo (1800).
    ('Duel', 'Reaper', {'lp': (5600, 3900)},
     '''you.hand: Breaker the Magical Warrior, Smashing Ground, Spirit Reaper
        you.m0: Cyber Dragon atk
        you.st0: Sakuretsu Armor set
        you.st1: Premature Burial set
        opp.m0: Kycoo the Ghost Destroyer atk
        opp.m1: Mystic Tomato set
        opp.st0: Torrential Tribute set
        opp.st1: Nobleman of Crossout set''',
     # (each play settles first: Breaker's Spell Counter arrives on a chain of its own after the summon, and its
     # menu has no Activate until then)
     '''point you.hand[Breaker the Magical Warrior]
        wait 30
        gif_start duel
        summon you.hand[Breaker the Magical Warrior] -> m1 atk
        settle
        wait 20
        activate you.m1
        target opp.st0
        settle
        wait 20
        battle
        attack you.m0 -> opp.m0
        settle
        wait 20
        rec_stop''',
     {'video': CLEAN}),
    # The full board, Goat staples only (usage/goat.json)
    ('Duel Board', 'Goat', {'lp': (6400, 5100)},
     '''you.m0: Breaker the Magical Warrior atk
        you.m1: Tribe-Infecting Virus atk
        you.m2: Gravekeeper's Spy set
        you.st0: Sakuretsu Armor set
        you.st1: Solemn Judgment set
        you.st2: Book of Moon set
        opp.m0: Black Luster Soldier - Envoy of Beginning atk
        opp.m1: Magician of Faith set
        opp.st0: Mirror Force set
        opp.st1: Scapegoat set''',
     '''point you.m0
        wait 30
        shot Duel Board'''),
    ('Chain List', 'Goat', None,
     '''you.hand: Mystical Space Typhoon
        you.st0: Dust Tornado set
        you.st1: Book of Moon set
        opp.m0: Black Luster Soldier - Envoy of Beginning atk
        opp.st0: Call of the Haunted set
        opp.st1: Mirror Force set''',
     # (a 3-link chain of three different cards: Dust Tornado answers Mystical Space Typhoon, Book of Moon answers
     # Dust Tornado; the list shows from about 75 frames after the last target)
     '''activate you.hand[Mystical Space Typhoon]
        target opp.st0
        when prompt response: activate you.st0
        target opp.st1
        when prompt response: activate you.st1
        target opp.m0
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
        view you.extra[Number 39: Utopia] 150
        extra_summon Number 39: Utopia from you.m0, you.m1 -> m0 atk
        point you.m0
        settle
        wait 20
        rec_stop''',
     {'video': SHOW_PICKS}),
    # (non-Effect materials, as viable as the demo allows: two Normal Monsters, Gemini Elf and Vorse Raider)
    ('Link', 'Goat', None,
     '''you.m0: Gemini Elf atk
        you.m1: Vorse Raider atk
        you.extra: Proxy Dragon''',
     '''point you.m0
        wait 50
        gif_start link
        extra_summon Proxy Dragon from you.m0, you.m1 -> emz0
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
    # Witch of the Black Forest revived by Call of the Haunted attacks the CPU's Sangan, and the CPU's Sakuretsu Armor
    # destroys it: Witch searches once (it used to search twice) and the Call goes with it. All on your turn, so the
    # Call stays in view (in the CPU's turn the board scrolls to its side, and its search list is not detected).
    ('Witch Call', 'Goat', None,
     '''you.st0: Call of the Haunted set
        you.gy: Witch of the Black Forest
        you.deck: Sangan, Spirit Reaper
        opp.m0: Sangan atk
        opp.st0: Sakuretsu Armor set''',
     '''activate you.st0
        pick you.gy[Witch of the Black Forest]
        settle
        point you.m0
        wait 30
        gif_start witch-call
        unfreeze
        battle
        attack you.m0 -> opp.m0
        when prompt list: pick you.deck[Sangan]
        settle
        point you.st0
        wait 90
        rec_stop''',
     {'video': {'lists': 'show', 'cards': 'show'}, 'freeze': False}),
    # ---- The new cards (video) ----
    ('New Cards', 'Goat', None,
     '''you.hand: Cyber Dragon, Breaker the Magical Warrior, Polymerization, Relinquished, Thousand-Eyes Idol
        you.extra: Thousand-Eyes Restrict
        opp.m0: Summoned Skull atk
        opp.m1: Mechanicalchaser atk
        opp.st0: Call of the Haunted set''',
     '''point you.hand[Cyber Dragon]
        wait 30
        gif_start new-cards
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
        options speed=always anims=hide battle=hide
        mp4_start ai-magic-cylinder
        battle
        attack you.m0 -> direct
        settle
        wait 20
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    # ---- The CPU at work (2026-11-17): one clip per behavior of AI_IMPROVEMENTS.md / AI_QUIRKS.md, each an MP4 in
    # screenshots/videos/ai-*.mp4; stitch_media.py `cpu-play` joins them at 2x speed into screenshots/gifs/cpu-play.gif.
    # The CPU plays for real (no freeze); its draws are set with opp.deck (the top card first). ----
    *ai_turn('AI Draw First', 'ai-draw-first',
             '''you.m0: Gemini Elf atk
                opp.hand: Pot of Greed, Graceful Charity, Upstart Goblin, Vorse Raider
                opp.deck: Sangan, Summoned Skull, Trap Hole, Mystic Tomato, Waboku, Dark Hole'''),
    # (exactly 3 Thunder Dragons: one in hand, two on top of the Deck; the rest of the Deck is Warrior's usage deck,
    # which runs none, where Goat's runs three more)
    *ai_turn('AI Thunder Dragon', 'ai-thunder-dragon',
             '''you.m0: Gemini Elf atk
                opp.hand: Thunder Dragon, Mystic Tomato
                opp.deck: Sangan, Thunder Dragon, Thunder Dragon''', fmt='Warrior'),
    *ai_turn('AI Dark Hole', 'ai-dark-hole',
             '''you.m0: Summoned Skull atk
                you.m1: Jinzo atk
                opp.m0: Mystic Tomato def
                opp.hand: Dark Hole, Vorse Raider
                opp.deck: Sangan'''),
    *ai_turn('AI Nobleman', 'ai-nobleman',
             '''you.m0: Gravekeeper's Spy set
                opp.hand: Nobleman of Crossout, Gemini Elf
                opp.deck: Sangan'''),
    *ai_turn('AI Steal Tribute', 'ai-steal-tribute',
             '''you.m0: Gemini Elf atk
                opp.m0: Mystic Tomato def
                opp.hand: Change of Heart, Summoned Skull
                opp.deck: Sangan'''),
    *ai_turn('AI Face Down Attack', 'ai-face-down-attack',
             '''you.m0: Gemini Elf set
                opp.m0: Vorse Raider atk
                opp.deck: Gemini Elf'''),
    *ai_turn('AI Exiled Force', 'ai-exiled-force',
             '''you.m0: Jinzo atk
                opp.hand: Exiled Force
                opp.deck: Sangan'''),
    *ai_turn('AI Virus', 'ai-virus',
             '''you.m0: Gemini Elf atk
                you.m1: Dark Magician atk
                opp.hand: Tribe-Infecting Virus, Mystic Tomato
                opp.deck: Sangan'''),
    *ai_turn('AI Enemy Controller', 'ai-enemy-controller',
             '''you.m0: Jinzo atk
                opp.m0: Mystic Tomato atk
                opp.hand: Enemy Controller
                opp.deck: Sangan'''),
    *ai_turn('AI Hane Hane', 'ai-hane-hane',
             '''you.m0: Summoned Skull atk
                opp.m0: Hane-Hane set
                opp.m1: Vorse Raider def
                opp.deck: Sangan'''),
    *ai_turn('AI Chaos Emperor', 'ai-chaos-emperor',
             '''you.hand: Gemini Elf, Vorse Raider, Dark Magician
                you.m0: Summoned Skull atk
                you.m1: Jinzo atk
                you.st0: Waboku set
                you.st1: Trap Hole set
                opp.hand: Chaos Emperor Dragon - Envoy of the End
                opp.gy: Sangan, Magician of Faith
                opp.deck: Mystic Tomato'''),
    # (you attack its face-down Man-Eater Bug with Gemini Elf: it destroys Summoned Skull, which has not attacked yet)
    ('AI Man Eater Bug', 'Goat', None,
     '''you.m0: Gemini Elf atk
        you.m1: Summoned Skull atk
        opp.m0: Man-Eater Bug set''',
     '''point you.m0
        wait 20
        options speed=always anims=hide battle=hide
        mp4_start ai-man-eater-bug
        battle
        attack you.m0 -> opp.m0
        settle
        wait 30
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    # (a Tribute upgrade that does not change its attacks: Gyakutenno Megami 1800/2000 over Aqua Madoor 1200/2000)
    *ai_turn('AI Tribute Upgrade', 'ai-tribute-upgrade',
             '''you.m0: Summoned Skull atk
                opp.m0: Aqua Madoor def
                opp.hand: Gyakutenno Megami
                opp.deck: Sangan'''),
    # (Kycoo attacks first while its attack deals damage, so its effect banishes from your Graveyard)
    *ai_turn('AI Kycoo', 'ai-kycoo',
             '''you.m0: Gemini Elf set
                you.gy: Summoned Skull, Jinzo, Dark Magician
                opp.m0: Kycoo the Ghost Destroyer atk
                opp.m1: Vorse Raider atk
                opp.deck: Sangan'''),
    ('AI Recruiter', 'Goat', None,
     '''you.m0: Vorse Raider atk
        opp.m0: Mystic Tomato atk
        opp.deck: Mystic Tomato, Mystic Tomato''',
     '''point you.m0
        wait 20
        options speed=always anims=hide battle=hide
        mp4_start ai-recruiter
        battle
        attack you.m0 -> opp.m0
        settle
        wait 30
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    ('AI Waboku', 'Goat', None,
     '''you.m0: Summoned Skull atk
        opp.m0: Gemini Elf atk
        opp.st0: Waboku set''',
     '''point you.m0
        wait 20
        options speed=always anims=hide battle=hide
        mp4_start ai-waboku
        battle
        attack you.m0 -> opp.m0
        settle
        wait 30
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    ('AI Trap Hole', 'Goat', None,
     '''you.hand: Breaker the Magical Warrior
        opp.st0: Trap Hole set''',
     '''point you.hand[Breaker the Magical Warrior]
        wait 20
        options speed=always anims=hide battle=hide
        mp4_start ai-trap-hole
        summon you.hand[Breaker the Magical Warrior] -> m0 atk
        settle
        wait 30
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    ('AI Solemn', 'Goat', {'lp': (8000, 4000)},
     '''you.hand: Vorse Raider
        opp.st0: Solemn Judgment set''',
     '''point you.hand[Vorse Raider]
        wait 20
        options speed=always anims=hide battle=hide
        mp4_start ai-solemn
        summon you.hand[Vorse Raider] -> m0 atk
        settle
        wait 30
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    # (the last clip: the steal wins the duel, so the clip ends a set time after the CPU plays it)
    ('AI Snatch Lethal', 'Goat', {'lp': (2400, 8000)},
     '''you.m0: Jinzo atk
        opp.hand: Snatch Steal
        opp.deck: Sangan''',
     '''point you.m0
        wait 20
        options speed=always anims=hide battle=hide
        mp4_start ai-snatch-lethal
        pass
        await Snatch Steal
        wait 240
        rec_stop''',
     {'video': AI_SHOW, 'freeze': False}),
    # ---- Settings (videos and a GIF) ----
    speed_turn('default'),
    speed_turn('always'),
    speed_turn('super'),
    ('Hide Anims', 'Goat', None,
     '''you.hand: Pot of Greed, Pot of Greed''',
     '''options anims=show
        point you.hand[0]
        wait 30
        gif_start hide-anims
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
    # Hold R: your five Set Spells/Traps, each shown in the info bar first, could answer every play of the CPU's turn
    # (Pot of Greed, Graceful Charity, Breaker and its effect); with R held no prompt comes.
    ('Hold R', 'Goat', None,
     '''you.st0: Sakuretsu Armor set
        you.st1: Dust Tornado set
        you.st2: Book of Moon set
        you.st3: Mystical Space Typhoon set
        you.st4: Solemn Judgment set
        opp.hand: Pot of Greed, Graceful Charity, Breaker the Magical Warrior
        opp.deck: Sangan, Spirit Reaper, Scapegoat, Mystic Tomato''',
     '''point you.st0
        gif_start hold-r
        wait 40
        point you.st1
        wait 40
        point you.st2
        wait 40
        point you.st3
        wait 40
        point you.st4
        wait 40
        pass
        hold R
        cpu_turn
        release R
        rec_stop''',
     {'video': {'chain': 'show', 'prompts': 'show'}, 'freeze': False}),
    # Hold L: the CPU's five Set Spells/Traps, each shown first (its five hand cards are not pointed at); with L held it
    # plays none of them
    # (it still draws).
    ('Hold L', 'Goat', None,
     '''you.m0: Gemini Elf atk
        opp.hand: Pot of Greed, Graceful Charity, Breaker the Magical Warrior, Heavy Storm, Snatch Steal
        opp.st0: Sakuretsu Armor set
        opp.st1: Dust Tornado set
        opp.st2: Book of Moon set
        opp.st3: Mystical Space Typhoon set
        opp.st4: Solemn Judgment set
        opp.deck: Sangan''',
     '''point opp.st0
        gif_start hold-l
        wait 35
        point opp.st1
        wait 35
        point opp.st2
        wait 35
        point opp.st3
        wait 35
        point opp.st4
        wait 35
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
