"""Exact scenarios (tools/testing_scenarios/exact.py): Scientist, Vampire and Chaos."""
from exact import cpu, player

OPP_TURN = {'turn': 'opp', 'turn_count': 3}

SCENARIOS = {
    'Breaker the Magical Warrior': [
        player('counter_then_destroy_spell_trap',
               setup='''
                   you.hand: Breaker the Magical Warrior
                   opp.st0: Call of the Haunted set
               ''',
               steps='''
                   summon you.hand[Breaker the Magical Warrior] -> m0 atk
                   check you.m0.atk = 1900
                   activate you.m0
                   target opp.st0
               ''',
               expect='''
                   opp.st0 = empty
                   opp.gy = Call of the Haunted
                   you.m0.atk = 1600
                   menu you.m0 lacks Activate
               '''),
        cpu('cpu_destroy_human_spell_trap', start=OPP_TURN,
            setup='''
                opp.hand: Breaker the Magical Warrior
                you.st0: Call of the Haunted set
            ''',
            steps='cpu_turn',
            expect='''
                you.st0 = empty
                you.gy = Call of the Haunted
                opp.monsters has Breaker the Magical Warrior
            '''),
    ],
    'Magical Scientist': [
        player('pay_1000_for_fusion',
               setup='''
                   you.m0: Magical Scientist atk
                   you.extra: Flame Swordsman, Thousand Dragon
               ''',
               steps='''
                   activate you.m0
                   pick you.extra[Flame Swordsman]
                   zone m1
                   position atk
                   check you.lp = 7000
                   check you.m1 = Flame Swordsman atk
                   battle
                   check cannot attack you.m1 -> direct
                   pass
               ''',
               expect='''
                   you.m1 = empty
                   you.extra = Flame Swordsman, Thousand Dragon
                   you.lp = 7000
               ''',
               note='Thousand Dragon (Level 7) is not offered: only Level 6 or lower.'),
        cpu('cpu_summon_fusion', start=OPP_TURN,
            setup='''
                opp.m0: Magical Scientist atk
                opp.extra: Flame Swordsman
                you.m0: Gemini Elf atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.lp <= 7000
                opp.extra has Flame Swordsman
            '''),
    ],
    'Tribe-Infecting Virus': [
        player('declare_type_destroy',
               setup='''
                   you.m0: Tribe-Infecting Virus atk
                   you.hand: Fissure
                   opp.m0: Gemini Elf atk
                   opp.m1: La Jinn the Mystical Genie of the Lamp atk
                   opp.m2: Summoned Skull atk
               ''',
               steps='''
                   activate you.m0
                   discard you.hand[Fissure]
                   declare Fiend
               ''',
               expect='''
                   opp.m0 = Gemini Elf atk
                   opp.m1 = empty
                   opp.m2 = empty
                   you.gy = Fissure
                   menu you.m0 lacks Activate
               '''),
        cpu('cpu_declare_best_type', start=OPP_TURN,
            setup='''
                opp.m0: Tribe-Infecting Virus atk
                opp.hand: Fissure
                you.m0: La Jinn the Mystical Genie of the Lamp atk
                you.m1: Summoned Skull atk
                you.m2: Gemini Elf atk
            ''',
            steps='cpu_turn',
            expect='''
                you.gy has Summoned Skull
                you.gy has La Jinn the Mystical Genie of the Lamp
                you.monsters lacks Summoned Skull
            '''),
    ],
    'Gilasaurus': [
        player('special_summon_opponent_revives',
               setup='''
                   you.hand: Gilasaurus
                   opp.gy: Summoned Skull
               ''',
               steps='special_summon you.hand[Gilasaurus] -> m0 atk',
               expect='''
                   you.m0 = Gilasaurus atk
                   opp.monsters = Summoned Skull
                   opp.gy count 0
               ''',
               note='The CPU always says Yes to its Graveyard summon.'),
        cpu('cpu_special_summon_human_revives', start=OPP_TURN,
            setup='''
                opp.hand: Gilasaurus
                you.gy: Summoned Skull
            ''',
            steps='''
                when prompt graveyard_summon: yes
                pick you.gy[Summoned Skull]
                zone m0
                position atk
            ''',
            expect='''
                opp.monsters has Gilasaurus
                you.m0 = Summoned Skull atk
                you.gy count 0
            '''),
    ],
    'Poison of the Old Man': [
        player('gain_1200',
               setup='you.hand: Poison of the Old Man',
               steps='''
                   activate you.hand[Poison of the Old Man]
                   choose "Gain 1200 LP"
               ''',
               expect='''
                   you.lp = 9200
                   opp.lp = 8000
               '''),
        player('burn_800',
               setup='you.hand: Poison of the Old Man',
               steps='''
                   activate you.hand[Poison of the Old Man]
                   choose "Inflict 800 damage"
               ''',
               expect='''
                   opp.lp = 7200
                   you.lp = 8000
               '''),
        cpu('cpu_burn', start=OPP_TURN,
            setup='opp.hand: Poison of the Old Man',
            steps='cpu_turn',
            expect='''
                you.lp = 7200
                opp.gy = Poison of the Old Man
            ''',
            note='With 8000 LP the CPU picks the damage (AiPoisonOfTheOldManChoice).'),
    ],
    'Reasoning': [
        player('excavate_special_summon',
               setup='''
                   you.hand: Reasoning
                   you.deck: Fissure, Trap Hole, Summoned Skull, Gemini Elf
               ''',
               steps='''
                   activate you.hand[Reasoning]
                   zone m0
                   position atk
               ''',
               expect='''
                   you.m0 = Summoned Skull atk
                   you.gy = Reasoning, Fissure, Trap Hole
               ''',
               note='The CPU declares Level 4; Summoned Skull is Level 6, so it is Special Summoned.'),
        cpu('cpu_human_declares_match', start=OPP_TURN,
            setup='''
                opp.hand: Reasoning
                opp.deck: Fissure, Gemini Elf
            ''',
            steps='''
                when prompt declare_level: declare level 4
            ''',
            expect='''
                opp.gy has Reasoning
                opp.gy has Fissure
                opp.gy has Gemini Elf
                opp.monsters lacks Gemini Elf
            '''),
    ],
    'Jowgen the Spiritualist': [
        player('destroy_special_summoned',
               setup='''
                   you.m0: Jowgen the Spiritualist atk
                   you.hand: Fissure, Gilasaurus
                   opp.m0: Summoned Skull atk ss
                   opp.m1: Gemini Elf atk
               ''',
               steps='''
                   check menu you.hand[Gilasaurus] lacks Special Summon
                   activate you.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.m1 = Gemini Elf atk
                   you.hand count 1
                   you.gy count 1
               ''',
               note='The discard is random (Fissure or Gilasaurus).'),
        cpu('cpu_lock_special_summon',
            setup='''
                opp.m0: Jowgen the Spiritualist atk
                you.hand: Gilasaurus
            ''',
            steps='check menu you.hand[Gilasaurus] lacks Special Summon',
            expect='you.hand = Gilasaurus'),
    ],
    'Spiritualism': [
        player('return_spell_trap',
               setup='''
                   you.hand: Spiritualism
                   opp.st0: Call of the Haunted set
               ''',
               steps='''
                   activate you.hand[Spiritualism]
                   target opp.st0
               ''',
               expect='''
                   opp.st0 = empty
                   opp.hand = Call of the Haunted
                   you.gy = Spiritualism
               '''),
        cpu('cpu_return_human_spell_trap', start=OPP_TURN,
            setup='''
                opp.hand: Spiritualism
                you.st0: Call of the Haunted set
            ''',
            steps='cpu_turn',
            expect='''
                you.st0 = empty
                you.hand = Call of the Haunted
            '''),
    ],
    'D.D. Warrior Lady': [
        player('banish_both',
               setup='''
                   you.m0: D.D. Warrior Lady atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   yes
               ''',
               expect='''
                   you.lp = 7000
                   opp.banished = Summoned Skull
                   you.banished = D.D. Warrior Lady
                   you.monsters count 0
                   opp.monsters count 0
               '''),
        cpu('cpu_banish_both',
            setup='''
                you.m0: Summoned Skull atk
                opp.m0: D.D. Warrior Lady atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                opp.lp = 7000
                you.banished = Summoned Skull
                opp.banished = D.D. Warrior Lady
            '''),
    ],
    'Sakuretsu Armor': [
        player('destroy_attacker', start=OPP_TURN,
               setup='''
                   you.st0: Sakuretsu Armor set
                   you.m0: Gemini Elf atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='when attack opp.m0 -> you.m0: activate you.st0',
               expect='''
                   opp.m0 = empty
                   opp.gy = Summoned Skull
                   you.m0 = Gemini Elf atk
                   you.lp = 8000
               '''),
        cpu('cpu_destroy_attacker',
            setup='''
                you.m0: Summoned Skull atk
                opp.st0: Sakuretsu Armor set
            ''',
            steps='''
                battle
                attack you.m0 -> direct
            ''',
            expect='''
                you.m0 = empty
                you.gy = Summoned Skull
                opp.lp = 8000
            '''),
    ],
    'Sasuke Samurai': [
        player('destroy_face_down',
               setup='''
                   you.m0: Sasuke Samurai atk
                   opp.m0: Summoned Skull set
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Summoned Skull
                   you.lp = 8000
               '''),
        cpu('cpu_destroy_face_down', start=OPP_TURN,
            setup='''
                opp.m0: Sasuke Samurai atk
                you.m0: Summoned Skull set
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.gy = Summoned Skull
            '''),
    ],
    'Amazoness Archer': [
        player('tribute_two_burn',
               setup='''
                   you.m0: Amazoness Archer atk
                   you.m1: Gemini Elf atk
                   you.m2: Mechanicalchaser atk
               ''',
               steps='''
                   activate you.m0
                   target you.m1
                   target you.m2
               ''',
               expect='''
                   opp.lp = 6800
                   you.gy = Gemini Elf, Mechanicalchaser
                   you.m0 = Amazoness Archer atk
               '''),
        cpu('cpu_tribute_two_burn', start=OPP_TURN,
            setup='''
                opp.m0: Amazoness Archer atk
                opp.m1: Gemini Elf atk
                opp.m2: Mechanicalchaser atk
            ''',
            steps='cpu_turn',
            expect='you.lp <= 6800'),
    ],
    'Vampire Lord': [
        player('declare_spell_on_damage',
               setup='''
                   you.m0: Vampire Lord atk
                   opp.deck: Fissure, Gemini Elf, Trap Hole
               ''',
               steps='''
                   battle
                   attack you.m0 -> direct
                   choose "Spell"
               ''',
               expect='''
                   opp.lp = 6000
                   opp.gy count 1
               ''',
               note="The CPU sends its first Spell/Trap of the declared type (Fissure is on top)."),
        player('revive_after_destroyed_by_effect',
               setup='''
                   you.m0: Gemini Elf atk
                   you.hand: Vampire Lord
                   opp.st0: Trap Hole set
               ''',
               steps='''
                   tribute_summon you.hand[Vampire Lord] -> m0 atk tributes you.m0
                   check you.gy has Vampire Lord
                   pass
                   cpu_turn
                   draw
                   position atk
               ''',
               expect='''
                   you.monsters has Vampire Lord
                   you.gy lacks Vampire Lord
                   opp.gy has Trap Hole
               ''',
               note='The CPU answers the summon with Trap Hole; Vampire Lord comes back in your next Standby Phase.'),
        cpu('cpu_declare_on_damage', start=OPP_TURN,
            setup='opp.m0: Vampire Lord atk',
            steps='''
                when prompt deck_pick: pick you.deck[0]
            ''',
            expect='''
                you.lp = 6000
                you.gy count 1
            ''',
            note='The CPU declares a type at random; pick the first card the list offers.'),
    ],
    'Chaos Emperor Dragon - Envoy of the End': [
        player('summon_and_wipe',
               setup='''
                   you.hand: Chaos Emperor Dragon - Envoy of the End, Gemini Elf
                   you.gy: Summoned Skull, Blue-Eyes White Dragon
                   you.st0: Trap Hole set
                   opp.m0: Gemini Elf atk
                   opp.st0: Trap Hole set
                   opp.hand: Fissure, Dark Hole
               ''',
               steps='''
                   special_summon you.hand[Chaos Emperor Dragon - Envoy of the End] -> m0 atk banish you.gy[Blue-Eyes White Dragon], you.gy[Summoned Skull]
                   activate you.m0
               ''',
               expect='''
                   you.lp = 7000
                   opp.lp = 5900
                   you.monsters count 0
                   opp.monsters count 0
                   you.hand count 0
                   opp.hand count 0
                   you.gy = Chaos Emperor Dragon - Envoy of the End, Trap Hole, Gemini Elf
                   opp.gy = Gemini Elf, Trap Hole, Fissure, Dark Hole
                   you.banished = Blue-Eyes White Dragon, Summoned Skull
               ''',
               note='7 cards are sent (the Dragon, 3 more on the field, 3 in the hands): 2100 damage.'),
        cpu('cpu_summon_and_wipe', start=OPP_TURN,
            setup='''
                opp.hand: Chaos Emperor Dragon - Envoy of the End
                opp.gy: Summoned Skull, Blue-Eyes White Dragon
                you.m0: Summoned Skull atk
                you.m1: Gemini Elf atk
                you.st0: Trap Hole set
                you.hand: Fissure, Dark Hole
            ''',
            steps='cpu_turn',
            expect='''
                opp.lp = 7000
                you.lp = 6200
                you.monsters count 0
                you.hand count 0
            ''',
            note='6 cards are sent (the Dragon, your 3 field cards and 2 hand cards): 1800 damage to you.'),
    ],
    'Dark Magician of Chaos': [
        player('banish_and_recover_spell',
               setup='''
                   you.hand: Dark Magician of Chaos
                   you.m0: Gemini Elf atk
                   you.m1: Mechanicalchaser atk
                   you.gy: Monster Reborn
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   tribute_summon you.hand[Dark Magician of Chaos] -> m0 atk tributes you.m0, you.m1
                   battle
                   attack you.m0 -> opp.m0
                   pass
                   yes
                   pick you.gy[Monster Reborn]
               ''',
               expect='''
                   opp.banished = Summoned Skull
                   opp.lp = 7700
                   you.hand = Monster Reborn
               '''),
        cpu('cpu_banish_by_battle', start=OPP_TURN,
            setup='''
                opp.m0: Dark Magician of Chaos atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.banished = Summoned Skull
                you.lp = 7700
            '''),
    ],
    'Chaos Sorcerer': [
        player('summon_and_banish',
               setup='''
                   you.hand: Chaos Sorcerer
                   you.gy: Summoned Skull, Blue-Eyes White Dragon
                   opp.m0: Mechanicalchaser atk
               ''',
               steps='''
                   special_summon you.hand[Chaos Sorcerer] -> m0 atk banish you.gy[Blue-Eyes White Dragon], you.gy[Summoned Skull]
                   activate you.m0
                   target opp.m0
                   battle
                   check cannot attack you.m0 -> direct
               ''',
               expect='''
                   opp.banished = Mechanicalchaser
                   you.banished = Blue-Eyes White Dragon, Summoned Skull
                   opp.lp = 8000
               '''),
        cpu('cpu_banish_human_monster', start=OPP_TURN,
            setup='''
                opp.m0: Chaos Sorcerer atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='you.banished = Summoned Skull'),
    ],
    'Tsukuyomi': [
        player('flip_set_target',
               setup='''
                   you.m0: Tsukuyomi set
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   flip_summon you.m0
                   target opp.m0
                   check opp.m0 = Summoned Skull set
                   pass
               ''',
               expect='''
                   you.hand = Tsukuyomi
                   you.monsters count 0
               ''',
               note='Tsukuyomi returns to the hand at the End Phase (Spirit).'),
        cpu('cpu_flip_set_target', start=OPP_TURN,
            setup='''
                opp.m0: Tsukuyomi set
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='you.m0 = Summoned Skull set'),
    ],
    'Cold Wave': [
        player('lock_spells_traps', start={'turn': 'you', 'turn_count': 2},
               setup='you.hand: Cold Wave, Pot of Greed',
               steps='''
                   activate you.hand[Cold Wave]
                   check menu you.hand[Pot of Greed] lacks Activate
                   pass
                   opp_turn
                   draw
               ''',
               expect='''
                   menu you.hand[Pot of Greed] has Activate
                   you.gy = Cold Wave
               ''',
               note='Activate it first thing in Main Phase 1. The lock lasts this turn and the next one.'),
        cpu('cpu_lock_spells_traps', start=OPP_TURN,
            setup='''
                opp.hand: Cold Wave
                you.hand: Pot of Greed
            ''',
            steps='''
                cpu_turn
                draw
            ''',
            expect='''
                opp.gy has Cold Wave
                menu you.hand[Pot of Greed] lacks Activate
            '''),
    ],
    'Thunder Nyan Nyan': [
        player('destroyed_by_non_light',
               setup='''
                   you.m0: Thunder Nyan Nyan atk
                   you.hand: Gemini Elf
               ''',
               steps='summon you.hand[Gemini Elf] -> m1 atk',
               expect='''
                   you.m0 = empty
                   you.gy = Thunder Nyan Nyan
                   you.m1 = Gemini Elf atk
               '''),
        player('face_down_does_not_count',
               setup='''
                   you.m0: Thunder Nyan Nyan atk
                   you.hand: Gemini Elf
               ''',
               steps='set you.hand[Gemini Elf] -> m1',
               expect='''
                   you.m0 = Thunder Nyan Nyan atk
                   you.m1 = Gemini Elf set
               '''),
        cpu('cpu_avoid_non_light', start=OPP_TURN,
            setup='''
                opp.m0: Thunder Nyan Nyan atk
                opp.hand: Gemini Elf
            ''',
            steps='cpu_turn',
            expect='opp.monsters has Thunder Nyan Nyan',
            note='The CPU does not summon Gemini Elf face up beside it (it may Set it).'),
    ],
}
