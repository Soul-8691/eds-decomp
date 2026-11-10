"""Exact scenarios (tools/testing_scenarios/exact.py): Fiber and Yata."""
from exact import cpu, player

OPP_TURN = {'turn': 'opp', 'turn_count': 3}

SCENARIOS = {
    'Exiled Force': [
        player('tribute_destroy_target',
               setup='''
                   you.m0: Exiled Force atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.m0
                   target opp.m0
               ''',
               expect='''
                   you.m0 = empty
                   you.gy = Exiled Force
                   opp.m0 = empty
                   opp.gy = Summoned Skull
               '''),
        cpu('cpu_tribute_destroy_stronger', start=OPP_TURN,
            setup='''
                opp.m0: Exiled Force atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.gy = Summoned Skull
                opp.gy has Exiled Force
            '''),
    ],
    'Fiber Jar': [
        player('flip_reset_both_players',
               setup='''
                   you.m0: Fiber Jar set
                   you.m1: Gemini Elf atk
                   you.st0: Trap Hole set
                   you.hand: Summoned Skull, Pot of Greed
                   you.gy: Mechanicalchaser
                   opp.m0: Gemini Elf atk
                   opp.hand: Fissure, Dark Hole, Mechanicalchaser
                   opp.gy: La Jinn the Mystical Genie of the Lamp
               ''',
               steps='flip_summon you.m0',
               expect='''
                   you.monsters count 0
                   you.spells count 0
                   opp.monsters count 0
                   you.gy count 0
                   opp.gy count 0
                   you.hand count 5
                   opp.hand count 5
               '''),
        cpu('cpu_flipped_by_attack',
            setup='''
                you.m0: Summoned Skull atk
                you.hand: Fissure
                opp.m0: Fiber Jar set
                opp.hand: Dark Hole
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                you.monsters count 0
                opp.monsters count 0
                you.hand count 5
                opp.hand count 5
            ''',
            note="Fiber Jar is flipped by the attack (DEF 500) and destroyed; its FLIP effect still resolves."),
    ],
    'Injection Fairy Lily': [
        player('pay_2000_for_3000',
               setup='''
                   you.m0: Injection Fairy Lily atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   yes
               ''',
               expect='''
                   you.lp = 6000
                   opp.lp = 7100
                   opp.m0 = empty
                   you.m0.atk = 400
               '''),
        cpu('cpu_pay_when_attacked',
            setup='''
                you.m0: Summoned Skull atk
                opp.m0: Injection Fairy Lily atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                opp.lp = 6000
                you.m0 = empty
                you.lp = 7100
                opp.m0 = Injection Fairy Lily atk
            '''),
    ],
    'Yata-Garasu': [
        player('skip_opponent_draw',
               setup='''
                   you.hand: Yata-Garasu
                   opp.hand: Fissure
               ''',
               steps='''
                   summon you.hand[Yata-Garasu] -> m0 atk
                   battle
                   attack you.m0 -> direct
                   check opp.lp = 7800
                   pass
                   check you.hand = Yata-Garasu
                   opp_turn
               ''',
               expect='''
                   you.monsters count 0
                   opp.hand = Fissure
               ''',
               note="The opponent skips its next Draw Phase, so its hand stays at 1 card."),
        cpu('cpu_skip_human_draw', start=OPP_TURN,
            setup='opp.hand: Yata-Garasu',
            steps='cpu_turn',
            expect='''
                you.lp = 7800
                opp.hand = Yata-Garasu
                you.hand count 0
            ''',
            note='Your next Draw Phase is skipped: no Draw is offered at the start of your turn.'),
    ],
    'Creature Swap': [
        player('swap_control',
               setup='''
                   you.hand: Creature Swap
                   you.m0: Mechanicalchaser atk
                   opp.m0: Summoned Skull atk
                   opp.m1: Gemini Elf atk
               ''',
               steps='''
                   activate you.hand[Creature Swap]
                   pick you.m0
               ''',
               expect='''
                   you.monsters = Gemini Elf
                   opp.monsters = Summoned Skull, Mechanicalchaser
                   menu you.m[Gemini Elf] lacks Change Position
               ''',
               note='The CPU gives its weakest monster (Gemini Elf).'),
        cpu('cpu_swap_weakest', start=OPP_TURN,
            setup='''
                opp.hand: Creature Swap
                opp.m0: Mechanicalchaser atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.monsters has Summoned Skull
                opp.gy has Creature Swap
            '''),
    ],
    'Asura Priest': [
        player('attack_every_monster',
               setup='''
                   you.m0: Asura Priest atk
                   opp.m0: Gemini Elf def
                   opp.m1: Mechanicalchaser def
                   opp.m2: Giant Soldier of Stone atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   attack you.m0 -> opp.m1
                   attack you.m0 -> opp.m2
                   check cannot attack you.m0 -> direct
                   pass
               ''',
               expect='''
                   opp.monsters count 0
                   opp.lp = 7600
                   you.hand = Asura Priest
               ''',
               note='Only the attack on Giant Soldier of Stone (300 ATK) deals damage: 1400. Asura Priest returns to the hand at '
                    'the End Phase.'),
        cpu('cpu_attack_every_monster', start=OPP_TURN,
            setup='''
                opp.m0: Asura Priest atk
                you.m0: Gemini Elf def
                you.m1: Mechanicalchaser def
            ''',
            steps='cpu_turn',
            expect='''
                you.monsters count 0
                opp.hand has Asura Priest
            '''),
    ],
    'Spear Dragon': [
        player('pierce_then_defense',
               setup='''
                   you.m0: Spear Dragon atk
                   opp.m0: Gemini Elf def
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.lp = 7000
                   you.m0 = Spear Dragon def
               '''),
        cpu('cpu_pierce', start=OPP_TURN,
            setup='''
                opp.m0: Spear Dragon atk
                you.m0: Gemini Elf def
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7000
                opp.m0 = Spear Dragon def
            '''),
    ],
    'Dark Ruler Ha Des': [
        player('negate_battle_destroyed_effects',
               setup='''
                   you.m0: Dark Ruler Ha Des atk
                   you.m1: Gemini Elf atk
                   opp.m0: Sangan atk
                   opp.m1: Man-Eater Bug set
               ''',
               steps='''
                   battle
                   attack you.m1 -> opp.m0
                   attack you.m0 -> opp.m1
               ''',
               expect='''
                   opp.monsters count 0
                   you.m0 = Dark Ruler Ha Des atk
                   you.m1 = Gemini Elf atk
                   opp.lp = 7100
                   opp.hand count 0
               ''',
               note="Sangan's search and Man-Eater Bug's FLIP effect do not activate (both destroyed by Fiends)."),
        cpu('cpu_negate_sangan', start=OPP_TURN,
            setup='''
                opp.m0: Dark Ruler Ha Des atk
                you.m0: Sangan atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 6550
                you.hand count 0
            '''),
    ],
    'Airknight Parshath': [
        player('pierce_and_draw',
               setup='''
                   you.m0: Airknight Parshath atk
                   you.deck: Mechanicalchaser
                   opp.m0: Gemini Elf def
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.lp = 7000
                   you.hand = Mechanicalchaser
               '''),
        cpu('cpu_pierce_and_draw', start=OPP_TURN,
            setup='''
                opp.m0: Airknight Parshath atk
                opp.deck: Mechanicalchaser
                you.m0: Gemini Elf def
            ''',
            steps='cpu_turn',
            expect='''
                you.lp = 7000
                you.m0 = empty
            '''),
    ],
    'Mirage of Nightmare': [
        player('draw_to_four_then_discard',
               setup='''
                   you.hand: Mirage of Nightmare, Gemini Elf
                   you.deck: Mechanicalchaser, La Jinn the Mystical Genie of the Lamp, Neo the Magic Swordsman, Fissure
               ''',
               steps='''
                   activate you.hand[Mirage of Nightmare]
                   pass
                   opp_turn
                   check you.hand = Gemini Elf, Mechanicalchaser, La Jinn the Mystical Genie of the Lamp, Neo the Magic Swordsman
                   draw
               ''',
               expect='''
                   you.hand count 2
                   you.gy count 3
                   you.spells has Mirage of Nightmare
               ''',
               note="In the opponent's Standby Phase you draw up to 4 (3 cards); after your own draw (Fissure, 5 "
                    "cards), your Standby Phase discards 3 at random."),
        cpu('cpu_draw_to_four', start=OPP_TURN,
            setup='''
                opp.st0: Mirage of Nightmare up
                opp.hand: Fissure
            ''',
            steps='''
                cpu_turn
                draw
            ''',
            expect='opp.hand count 4',
            note="The CPU draws up to 4 in your Standby Phase (after your own draw)."),
    ],
    'Spirit Reaper': [
        player('discard_on_direct_attack',
               setup='''
                   you.m0: Spirit Reaper atk
                   opp.hand: Fissure
               ''',
               steps='''
                   battle
                   attack you.m0 -> direct
               ''',
               expect='''
                   opp.lp = 7700
                   opp.hand count 0
                   opp.gy = Fissure
               '''),
        player('survive_battle',
               setup='''
                   you.m0: Spirit Reaper atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   you.m0 = Spirit Reaper atk
                   you.lp = 5800
               '''),
        cpu('cpu_discard_human_card', start=OPP_TURN,
            setup='''
                opp.m0: Spirit Reaper atk
                you.hand: Fissure
            ''',
            steps='cpu_turn',
            expect='''
                you.lp = 7700
                you.hand count 0
                you.gy = Fissure
            '''),
    ],
    'Don Zaloog': [
        player('discard_random',
               setup='''
                   you.m0: Don Zaloog atk
                   opp.hand: Fissure, Trap Hole
               ''',
               steps='''
                   battle
                   attack you.m0 -> direct
                   yes
               ''',
               expect='''
                   opp.lp = 6600
                   opp.hand count 1
                   opp.gy count 1
               '''),
        player('send_top_two',
               setup='''
                   you.m0: Don Zaloog atk
                   opp.hand: Fissure, Trap Hole
                   opp.deck: Summoned Skull, Gemini Elf
               ''',
               steps='''
                   battle
                   attack you.m0 -> direct
                   no
                   yes
               ''',
               expect='''
                   opp.lp = 6600
                   opp.hand count 2
                   opp.gy = Summoned Skull, Gemini Elf
               '''),
        cpu('cpu_discard_random', start=OPP_TURN,
            setup='''
                opp.m0: Don Zaloog atk
                you.hand: Fissure, Trap Hole
            ''',
            steps='cpu_turn',
            expect='''
                you.lp = 6600
                you.hand count 1
            '''),
    ],
    'Reinforcement of the Army': [
        player('search_warrior',
               setup='''
                   you.hand: Reinforcement of the Army
                   you.deck: Gemini Elf, Celtic Guardian, Summoned Skull
               ''',
               steps='''
                   activate you.hand[Reinforcement of the Army]
                   pick you.deck[Celtic Guardian]
               ''',
               expect='''
                   you.hand = Celtic Guardian
                   you.gy = Reinforcement of the Army
               '''),
        cpu('cpu_search_warrior', start=OPP_TURN,
            setup='''
                opp.hand: Reinforcement of the Army
                opp.deck: Celtic Guardian
            ''',
            steps='cpu_turn',
            expect='opp.gy has Reinforcement of the Army'),
    ],
    "Gravekeeper's Spy": [
        player('flip_summon_gravekeeper',
               setup='''
                   you.m0: Gravekeeper's Spy set
                   you.deck: Gravekeeper's Spy, Gemini Elf
               ''',
               steps='''
                   flip_summon you.m0
                   yes
                   pick you.deck[Gravekeeper's Spy]
                   zone m1
               ''',
               expect='''
                   you.m0 = Gravekeeper's Spy atk
                   you.m1 = Gravekeeper's Spy atk
               ''',
               note="Gravekeeper's Spy is the only Gravekeeper's monster in the ROM."),
        cpu('cpu_flipped_by_attack',
            setup='''
                you.m0: Gemini Elf atk
                opp.m0: Gravekeeper's Spy set
                opp.deck: Gravekeeper's Spy
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                you.lp = 7900
                opp.monsters = Gravekeeper's Spy, Gravekeeper's Spy
            '''),
    ],
    'Book of Moon': [
        player('face_down_target',
               setup='''
                   you.hand: Book of Moon
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Book of Moon]
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = Summoned Skull set
                   you.gy = Book of Moon
               '''),
        cpu('cpu_clear_the_way', start=OPP_TURN,
            setup='''
                opp.hand: Book of Moon
                opp.m0: Gemini Elf atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Book of Moon
                you.m0 = empty
            ''',
            note='Book of Moon turns Summoned Skull face down (DEF 1200), then La Jinn (1800) destroys it.'),
    ],
    'King Tiger Wanghu': [
        player('destroy_weak_summon',
               setup='''
                   you.m0: King Tiger Wanghu atk
                   you.hand: Giant Soldier of Stone
               ''',
               steps='summon you.hand[Giant Soldier of Stone] -> m1 atk',
               expect='''
                   you.m1 = empty
                   you.gy = Giant Soldier of Stone
                   you.m0 = King Tiger Wanghu atk
               '''),
        cpu('cpu_wanghu_on_human_summon',
            setup='''
                opp.m0: King Tiger Wanghu atk
                you.hand: Giant Soldier of Stone
            ''',
            steps='summon you.hand[Giant Soldier of Stone] -> m0 atk',
            expect='''
                you.m0 = empty
                you.gy = Giant Soldier of Stone
            '''),
    ],
    'Newdoria': [
        player('destroy_on_battle_destruction',
               setup='''
                   you.m0: Newdoria atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   target opp.m0
               ''',
               expect='''
                   you.lp = 6700
                   you.gy = Newdoria
                   opp.m0 = empty
                   opp.gy = Summoned Skull
               '''),
        cpu('cpu_destroy_strongest',
            setup='''
                you.m0: Summoned Skull atk
                you.m1: Gemini Elf atk
                opp.m0: Newdoria atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                opp.lp = 6700
                you.m0 = empty
                you.m1 = Gemini Elf atk
            '''),
    ],
    'Metamorphosis': [
        player('tribute_for_same_level_fusion',
               setup='''
                   you.hand: Metamorphosis
                   you.m0: Dark Magician atk
                   you.extra: Thousand Dragon, Flame Swordsman
               ''',
               steps='''
                   activate you.hand[Metamorphosis]
                   target you.m0
                   pick you.extra[Thousand Dragon]
                   zone m0
                   position atk
               ''',
               expect='''
                   you.m0 = Thousand Dragon atk
                   you.gy = Metamorphosis, Dark Magician
                   you.extra = Flame Swordsman
               '''),
        cpu('cpu_highest_atk_fusion', start=OPP_TURN,
            setup='''
                opp.hand: Metamorphosis
                opp.m0: Dark Magician atk
                opp.extra: Thousand Dragon, Gaia the Dragon Champion
            ''',
            steps='cpu_turn',
            expect='''
                opp.monsters has Gaia the Dragon Champion
                opp.gy has Dark Magician
            '''),
    ],
    'Cure Mermaid': [
        player('gain_800_standby',
               setup='you.m0: Cure Mermaid atk',
               steps='''
                   pass
                   opp_turn
                   draw
               ''',
               expect='you.lp = 8800'),
        cpu('cpu_gain_800_standby',
            setup='opp.m0: Cure Mermaid atk',
            steps='''
                pass
                cpu_turn
            ''',
            expect='opp.lp >= 8800'),
    ],
    'Raigeki Break': [
        player('discard_destroy_card',
               setup='''
                   you.st0: Raigeki Break set
                   you.hand: Gemini Elf
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.st0
                   discard you.hand[Gemini Elf]
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Summoned Skull
                   you.gy = Raigeki Break, Gemini Elf
               '''),
        cpu('cpu_discard_destroy', start=OPP_TURN,
            setup='''
                opp.st0: Raigeki Break set
                opp.hand: Fissure
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Raigeki Break
                you.m0 = empty
            '''),
    ],
    'Trap Dustshoot': [
        player('return_hand_monster',
               setup='''
                   you.st0: Trap Dustshoot set
                   opp.hand: Summoned Skull, Fissure, Trap Hole, Dark Hole
               ''',
               steps='''
                   activate you.st0
                   pick opp.hand[Summoned Skull]
               ''',
               expect='''
                   opp.hand = Fissure, Trap Hole, Dark Hole
                   you.gy = Trap Dustshoot
               '''),
        cpu('cpu_return_first_monster', start=OPP_TURN,
            setup='''
                opp.st0: Trap Dustshoot set
                you.hand: Summoned Skull, Fissure, Trap Hole, Dark Hole
            ''',
            steps='cpu_turn',
            expect='''
                you.hand = Fissure, Trap Hole, Dark Hole
                opp.gy has Trap Dustshoot
            '''),
    ],
    'Blast with Chain': [
        player('equip_then_destroy_when_destroyed',
               setup='''
                   you.st0: Blast with Chain set
                   you.hand: Mystical Space Typhoon
                   you.m0: Gemini Elf atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.st0
                   target you.m0
                   check you.m0.atk = 2400
                   activate you.hand[Mystical Space Typhoon]
                   target you.st0
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Summoned Skull
                   you.gy = Blast with Chain, Mystical Space Typhoon
                   you.m0.atk = 1900
               '''),
        cpu('cpu_equip_against_attack',
            setup='''
                you.m0: Gemini Elf atk
                opp.st0: Blast with Chain set
                opp.m0: Mechanicalchaser atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                you.m0 = empty
                you.lp = 7550
                opp.m0 = Mechanicalchaser atk
                opp.m0.atk = 2350
            '''),
    ],
    'Gearfried the Iron Knight': [
        player('destroy_equip',
               setup='''
                   you.m0: Gearfried the Iron Knight atk
                   you.hand: Mage Power
               ''',
               steps='''
                   activate you.hand[Mage Power]
                   target you.m0
               ''',
               expect='''
                   you.gy = Mage Power
                   you.m0 = Gearfried the Iron Knight atk
                   you.m0.atk = 1800
               '''),
        cpu('cpu_attack', start=OPP_TURN,
            setup='''
                opp.m0: Gearfried the Iron Knight atk
                you.m0: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7900
            '''),
    ],
}
