"""Exact scenarios (tools/testing_scenarios/exact.py): Yugi-Kaiba to Joey-Pegasus."""
from exact import cpu, player

OPP_TURN = {'turn': 'opp', 'turn_count': 3}

SCENARIOS = {
    'Card Destruction': [
        player('discard_and_draw',
               setup='''
                   you.hand: Card Destruction, Dark Hole, Battle Ox
                   you.deck: Summoned Skull, Celtic Guardian, Fissure
                   opp.hand: Trap Hole, Fissure, Battle Ox, La Jinn the Mystical Genie of the Lamp
                   opp.deck: Dark Magician, Celtic Guardian, Man-Eater Bug, Swords of Revealing Light
               ''',
               steps='activate you.hand[Card Destruction]',
               expect='''
                   you.hand = Summoned Skull, Celtic Guardian
                   you.gy = Card Destruction, Dark Hole, Battle Ox
                   opp.hand = Dark Magician, Celtic Guardian, Man-Eater Bug, Swords of Revealing Light
                   opp.gy = Trap Hole, Fissure, Battle Ox, La Jinn the Mystical Genie of the Lamp
               '''),
        cpu('cpu_discard_and_draw', start=OPP_TURN,
            note='The CPU plays it when the human holds more cards than it would discard.',
            setup='''
                opp.hand: Card Destruction, Battle Ox
                you.hand: Summoned Skull, Dark Hole, Fissure, Trap Hole, Celtic Guardian
                you.deck: Battle Ox, Celtic Guardian, Man-Eater Bug, Fissure, Swords of Revealing Light
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Card Destruction
                you.gy = Summoned Skull, Dark Hole, Fissure, Trap Hole, Celtic Guardian
                you.hand = Battle Ox, Celtic Guardian, Man-Eater Bug, Fissure, Swords of Revealing Light
            '''),
    ],
    'Soul Exchange': [
        player('tribute_opponent_monster',
               setup='''
                   you.hand: Soul Exchange, Summoned Skull
                   opp.m0: Battle Ox atk
               ''',
               steps='''
                   activate you.hand[Soul Exchange]
                   target opp.m0
                   tribute_summon you.hand[Summoned Skull] -> m0 atk tributes opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Battle Ox
                   you.m0 = Summoned Skull atk
                   you.gy = Soul Exchange
                   battle_phase blocked
               '''),
        cpu('cpu_tribute_opponent_monster', start=OPP_TURN,
            setup='''
                opp.hand: Soul Exchange, Summoned Skull
                you.m0: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.gy = Battle Ox
                opp.monsters = Summoned Skull
                opp.gy = Soul Exchange
                you.lp = 8000
            '''),
    ],
    'Thousand-Eyes Restrict': [
        player('absorb_and_attack',
               setup='''
                   you.m0: Thousand-Eyes Restrict atk
                   opp.m0: Summoned Skull atk
                   opp.m1: Battle Ox atk
               ''',
               steps='''
                   activate you.m0
                   target opp.m0
                   check you.m0.atk = 2500
                   check you.m0.def = 1200
                   battle
                   attack you.m0 -> opp.m1
               ''',
               expect='''
                   opp.monsters count 0
                   opp.gy = Battle Ox
                   opp.lp = 7200
                   menu you.m0 lacks Activate
               '''),
        cpu('cpu_absorb', start=OPP_TURN,
            setup='''
                opp.m0: Thousand-Eyes Restrict atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 5500
            ''',
            note='It absorbs Summoned Skull (2500 ATK), then attacks directly.'),
    ],
    'The Fiend Megacyber': [
        player('special_summon_from_hand',
               setup='''
                   you.hand: The Fiend Megacyber
                   opp.m0: Battle Ox atk
                   opp.m1: La Jinn the Mystical Genie of the Lamp atk
               ''',
               steps='special_summon you.hand[The Fiend Megacyber] -> m0 atk',
               expect='you.m0 = The Fiend Megacyber atk'),
        player('not_with_one_monster_difference',
               setup='''
                   you.hand: The Fiend Megacyber
                   you.m0: Celtic Guardian atk
                   opp.m0: Battle Ox atk
                   opp.m1: La Jinn the Mystical Genie of the Lamp atk
               ''',
               steps='check menu you.hand[The Fiend Megacyber] lacks Special Summon',
               expect='you.hand = The Fiend Megacyber'),
        cpu('cpu_special_summon', start=OPP_TURN,
            setup='''
                opp.hand: The Fiend Megacyber
                you.m0: Battle Ox atk
                you.m1: Celtic Guardian atk
            ''',
            steps='cpu_turn',
            expect='opp.monsters has The Fiend Megacyber'),
    ],
    'Goblin Attack Force': [
        player('mandatory_follow_up',
               setup='''
                   you.m0: Goblin Attack Force atk
                   opp.m0: Celtic Guardian atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   main2
                   check you.m0 = Goblin Attack Force def
                   pass
                   opp_turn
                   draw
                   check menu you.m0 lacks Change Position
               ''',
               expect='''
                   opp.lp = 7100
                   opp.gy = Celtic Guardian
                   you.m0 = Goblin Attack Force def
               '''),
        cpu('cpu_attack_then_defense', start=OPP_TURN,
            setup='''
                opp.m0: Goblin Attack Force atk
                you.m0: Celtic Guardian atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7100
                opp.m0 = Goblin Attack Force def
            '''),
    ],
    'Solemn Wishes': [
        player('gain_per_draw',
               setup='''
                   you.st0: Solemn Wishes set
                   you.hand: Pot of Greed
                   you.deck: Battle Ox, Celtic Guardian, Fissure
               ''',
               steps='''
                   activate you.st0
                   activate you.hand[Pot of Greed]
                   check you.lp = 8500
                   check you.hand = Battle Ox, Celtic Guardian
                   pass
                   opp_turn
                   draw
               ''',
               expect='''
                   you.lp = 9000
                   you.hand = Battle Ox, Celtic Guardian, Fissure
                   you.st0 = Solemn Wishes up
               ''',
               note='One gain per draw action: Pot of Greed draws 2 cards for 500 LP.'),
        cpu('cpu_activate_and_gain', start=OPP_TURN,
            setup='''
                opp.st0: Solemn Wishes set
            ''',
            steps='''
                cpu_turn
                check opp.st0 = Solemn Wishes up
                draw
                pass
                cpu_turn
            ''',
            expect='opp.lp >= 8500'),
    ],
    'Limiter Removal': [
        player('double_machine_atk',
               setup='''
                   you.hand: Limiter Removal
                   you.m0: Jinzo atk
                   you.m1: Battle Ox atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Limiter Removal]
                   check you.m0.atk = 4800
                   check you.m1.atk = 1700
                   battle
                   attack you.m0 -> opp.m0
                   check opp.lp = 5700
                   pass
               ''',
               expect='''
                   you.m0 = empty
                   you.gy = Limiter Removal, Jinzo
                   you.m1 = Battle Ox atk
                   opp.m0 = empty
               '''),
        cpu('cpu_double_machine_atk', start=OPP_TURN,
            setup='''
                opp.hand: Limiter Removal
                opp.m0: Jinzo atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Limiter Removal
                opp.gy has Jinzo
                you.m0 = empty
                you.lp = 5700
            '''),
    ],
    '4-Starred Ladybug of Doom': [
        player('flip_destroy_level_4',
               setup='''
                   you.m0: 4-Starred Ladybug of Doom set
                   opp.m0: Battle Ox atk
                   opp.m1: La Jinn the Mystical Genie of the Lamp atk
                   opp.m2: Summoned Skull atk
                   opp.m3: Man-Eater Bug set
               ''',
               steps='flip_summon you.m0',
               expect='''
                   opp.m0 = empty
                   opp.m1 = empty
                   opp.m2 = Summoned Skull atk
                   opp.m3 = Man-Eater Bug set
                   opp.gy = Battle Ox, La Jinn the Mystical Genie of the Lamp
                   you.m0 = 4-Starred Ladybug of Doom atk
               ''',
               note='A face-down monster has no Level, so Man-Eater Bug stays.'),
        cpu('cpu_flip_destroy_level_4', start=OPP_TURN,
            setup='''
                opp.m0: 4-Starred Ladybug of Doom set
                you.m0: Battle Ox atk
                you.m1: Celtic Guardian atk
            ''',
            steps='cpu_turn',
            expect='''
                you.monsters count 0
                you.gy = Battle Ox, Celtic Guardian
            '''),
    ],
    'Kycoo the Ghost Destroyer': [
        player('banish_on_battle_damage',
               setup='''
                   you.m0: Kycoo the Ghost Destroyer atk
                   opp.gy: Summoned Skull, Battle Ox, Sangan
               ''',
               steps='''
                   battle
                   attack you.m0 -> direct
                   yes
                   pick opp.gy[Summoned Skull]
                   pick opp.gy[Sangan]
               ''',
               expect='''
                   opp.lp = 6200
                   opp.banished = Summoned Skull, Sangan
                   opp.gy = Battle Ox
               '''),
        cpu('cpu_banish_on_battle_damage', start=OPP_TURN,
            setup='''
                opp.m0: Kycoo the Ghost Destroyer atk
                you.gy: Summoned Skull, Battle Ox
            ''',
            steps='cpu_turn',
            expect='''
                you.lp = 6200
                you.banished count 2
                you.gy count 0
            '''),
    ],
    'Bazoo the Soul-Eater': [
        player('banish_three_for_atk',
               setup='''
                   you.m0: Bazoo the Soul-Eater atk
                   you.gy: Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan, Summoned Skull
               ''',
               steps='''
                   activate you.m0
                   yes
                   pick you.gy[Battle Ox]
                   yes
                   pick you.gy[La Jinn the Mystical Genie of the Lamp]
                   yes
                   pick you.gy[Sangan]
               ''',
               expect='''
                   you.m0.atk = 2500
                   you.banished = Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan
                   you.gy = Summoned Skull
                   menu you.m0 lacks Activate
               '''),
        cpu('cpu_banish_weakest', start=OPP_TURN,
            setup='''
                opp.m0: Bazoo the Soul-Eater atk
                opp.gy: Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan, Kuriboh
                you.m0: Jinzo atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.banished = Kuriboh, Sangan, Battle Ox
                opp.gy = La Jinn the Mystical Genie of the Lamp
                opp.m0.atk = 2500
                you.m0 = empty
                you.lp = 7900
            ''',
            note='BazooCpuResolve banishes its 3 weakest Graveyard monsters; 2500 ATK then beats Jinzo.'),
    ],
    'Torrential Tribute': [
        player('destroy_all_on_summon',
               setup='''
                   you.m0: Mystic Tomato atk
                   you.st0: Torrential Tribute set
                   you.hand: Battle Ox
                   opp.m0: Summoned Skull atk
                   opp.m1: Man-Eater Bug set
               ''',
               steps='''
                   summon you.hand[Battle Ox] -> m1 atk
                   when prompt response: activate you.st0
               ''',
               expect='''
                   you.monsters count 0
                   opp.monsters count 0
                   you.gy = Mystic Tomato, Battle Ox, Torrential Tribute
                   opp.gy = Summoned Skull, Man-Eater Bug
               ''',
               note='Destroyed by an effect: Mystic Tomato does not trigger, and the Set Man-Eater Bug is not flipped.'),
        cpu('cpu_on_human_summon',
            setup='''
                opp.st0: Torrential Tribute set
                you.m0: Summoned Skull atk
                you.m1: La Jinn the Mystical Genie of the Lamp atk
                you.hand: Battle Ox
            ''',
            steps='summon you.hand[Battle Ox] -> m2 atk',
            expect='''
                you.monsters count 0
                you.gy = Summoned Skull, La Jinn the Mystical Genie of the Lamp, Battle Ox
                opp.gy = Torrential Tribute
            '''),
    ],
    'Jar of Greed': [
        player('draw_one',
               setup='''
                   you.st0: Jar of Greed set
                   you.deck: Battle Ox
               ''',
               steps='activate you.st0',
               expect='''
                   you.hand = Battle Ox
                   you.gy = Jar of Greed
               '''),
        cpu('cpu_draw_one', start=OPP_TURN,
            setup='''
                opp.st0: Jar of Greed set
                opp.deck: Battle Ox
            ''',
            steps='cpu_turn',
            expect='opp.gy has Jar of Greed'),
    ],
    'Skull Lair': [
        player('banish_level_count',
               setup='''
                   you.st0: Skull Lair set
                   you.gy: Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan, Mystic Tomato
                   opp.m0: Sangan atk
               ''',
               steps='''
                   activate you.st0
                   target opp.m0
                   pick you.gy[Battle Ox]
                   pick you.gy[La Jinn the Mystical Genie of the Lamp]
                   pick you.gy[Sangan]
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Sangan
                   you.banished = Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan
                   you.gy = Mystic Tomato
                   you.st0 = Skull Lair up
               ''',
               note='Sangan is Level 3, so 3 Graveyard monsters are banished.'),
        cpu('cpu_destroy_strong_monster', start=OPP_TURN,
            setup='''
                opp.st0: Skull Lair set
                opp.gy: Battle Ox, La Jinn the Mystical Genie of the Lamp, Sangan, Kuriboh
                you.m0: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                opp.banished count 4
                opp.st0 = Skull Lair up
            '''),
    ],
    'Magic Cylinder': [
        player('reflect_attack', start=OPP_TURN,
               setup='''
                   you.st0: Magic Cylinder set
                   opp.m0: Summoned Skull atk
               ''',
               steps='when attack opp.m0 -> direct: activate you.st0',
               expect='''
                   opp.lp = 5500
                   you.lp = 8000
                   you.gy = Magic Cylinder
                   opp.m0 = Summoned Skull atk
               '''),
        cpu('cpu_reflect_attack',
            setup='''
                you.m0: Summoned Skull atk
                opp.st0: Magic Cylinder set
            ''',
            steps='''
                battle
                attack you.m0 -> direct
            ''',
            expect='''
                you.lp = 5500
                opp.lp = 8000
                opp.gy = Magic Cylinder
            '''),
    ],
    'St. Joan': [
        player('fusion_summon',
               setup='''
                   you.hand: Polymerization, The Forgiving Maiden, Darklord Marie
                   you.extra: St. Joan
               ''',
               steps='fusion_summon St. Joan from you.hand[The Forgiving Maiden], you.hand[Darklord Marie] -> m0 atk',
               expect='''
                   you.m0 = St. Joan atk
                   you.m0.atk = 2800
                   you.gy = Polymerization, The Forgiving Maiden, Darklord Marie
               '''),
        cpu('cpu_attack', start=OPP_TURN,
            setup='''
                opp.m0: St. Joan atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7700
            '''),
    ],
    'United We Stand': [
        player('equip_per_face_up_monster',
               setup='''
                   you.hand: United We Stand
                   you.m0: Battle Ox atk
                   you.m1: La Jinn the Mystical Genie of the Lamp atk
                   you.m2: Mystic Tomato set
               ''',
               steps='''
                   activate you.hand[United We Stand]
                   target you.m0
                   check you.m0.atk = 3300
                   check you.m0.def = 2600
                   flip_summon you.m2
               ''',
               expect='''
                   you.m0.atk = 4100
                   you.m0.def = 3400
               ''',
               note='+800 ATK/DEF for each face-up monster you control: 2, then 3 after the Flip Summon.'),
        cpu('cpu_equip', start=OPP_TURN,
            setup='''
                opp.hand: United We Stand
                opp.m0: Battle Ox atk
                opp.m1: La Jinn the Mystical Genie of the Lamp atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.spells has United We Stand
                you.m0 = empty
            '''),
    ],
    'Mage Power': [
        player('equip_per_spell_trap',
               setup='''
                   you.hand: Mage Power
                   you.m0: Battle Ox atk
                   you.st0: Trap Hole set
                   you.st1: Call of the Haunted set
               ''',
               steps='''
                   activate you.hand[Mage Power]
                   target you.m0
               ''',
               expect='''
                   you.m0.atk = 3200
                   you.m0.def = 2500
               ''',
               note='+500 for each of your Spells/Traps, Mage Power included: 3.'),
        cpu('cpu_equip', start=OPP_TURN,
            setup='''
                opp.hand: Mage Power
                opp.m0: Battle Ox atk
                opp.st0: Trap Hole set
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.m0.atk = 2700
                you.m0 = empty
                you.lp = 7800
            '''),
    ],
    'Zombyra the Dark': [
        player('lose_atk_after_destroying',
               setup='''
                   you.m0: Zombyra the Dark atk
                   opp.m0: Battle Ox atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.lp = 7600
                   you.m0.atk = 1900
               '''),
        player('no_direct_attack',
               setup='you.m0: Zombyra the Dark atk',
               steps='battle',
               expect='''
                   cannot attack you.m0 -> direct
                   opp.lp = 8000
               '''),
        cpu('cpu_attack', start=OPP_TURN,
            setup='''
                opp.m0: Zombyra the Dark atk
                you.m0: Celtic Guardian atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7300
                opp.m0.atk = 1900
            '''),
    ],
    'Humanoid Worm Drake': [
        player('fusion_summon',
               setup='''
                   you.hand: Polymerization, Worm Drake, Humanoid Slime
                   you.extra: Humanoid Worm Drake
               ''',
               steps='fusion_summon Humanoid Worm Drake from you.hand[Worm Drake], you.hand[Humanoid Slime] -> m0 atk',
               expect='''
                   you.m0 = Humanoid Worm Drake atk
                   you.gy = Polymerization, Worm Drake, Humanoid Slime
               '''),
        cpu('cpu_attack', start=OPP_TURN,
            setup='''
                opp.m0: Humanoid Worm Drake atk
                you.m0: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7500
            '''),
    ],
    'Scapegoat': [
        player('four_sheep_no_summon',
               setup='you.hand: Scapegoat, Battle Ox',
               steps='activate you.hand[Scapegoat]',
               expect='''
                   you.monsters = Sheep Token, Sheep Token, Sheep Token, Sheep Token
                   menu you.hand[Battle Ox] lacks Summon
                   menu you.hand[Battle Ox] has Set
               ''',
               note='The 4 Sheep Tokens are in Defense Position.'),
        cpu('cpu_block_direct_attack',
            setup='''
                you.m0: Summoned Skull atk
                opp.st0: Scapegoat set
            ''',
            steps='''
                battle
                attack you.m0 -> direct
                when replay: cancel
            ''',
            expect='''
                opp.monsters = Sheep Token, Sheep Token, Sheep Token, Sheep Token
                opp.lp = 8000
                opp.gy = Scapegoat
            '''),
    ],
    'Offerings to the Doomed': [
        player('destroy_and_skip_draw',
               setup='''
                   you.hand: Offerings to the Doomed
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Offerings to the Doomed]
                   target opp.m0
                   check opp.m0 = empty
                   pass
                   opp_turn
               ''',
               expect='''
                   opp.gy = Summoned Skull
                   you.gy = Offerings to the Doomed
                   you.hand count 0
               ''',
               note='Your next Draw Phase is skipped: no Draw is offered, and the hand stays empty.'),
        cpu('cpu_destroy', start=OPP_TURN,
            setup='''
                opp.hand: Offerings to the Doomed
                opp.m0: Battle Ox atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                opp.gy has Offerings to the Doomed
            '''),
    ],
}
