"""Exact scenarios (tools/testing_scenarios/exact.py): Warrior and Goat."""
from exact import cpu, player

OPP_TURN = {'turn': 'opp', 'turn_count': 3}

SCENARIOS = {
    'Cipher Soldier': [
        player('bonus_against_warrior',
               setup='''
                   you.m0: Cipher Soldier atk
                   opp.m0: Celtic Guardian atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.lp = 6050
                   you.m0.atk = 1350
               ''',
               note='3350 ATK during damage calculation only.'),
        cpu('cpu_bonus_when_attacked',
            setup='''
                you.m0: Celtic Guardian atk
                opp.m0: Cipher Soldier atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                you.m0 = empty
                you.lp = 6050
                opp.m0 = Cipher Soldier atk
            '''),
    ],
    'Mobius the Frost Monarch': [
        player('destroy_two_spell_traps',
               setup='''
                   you.hand: Mobius the Frost Monarch
                   you.m0: Battle Ox atk
                   opp.st0: Call of the Haunted set
                   opp.st1: Magic Jammer set
               ''',
               steps='''
                   tribute_summon you.hand[Mobius the Frost Monarch] -> m0 atk tributes you.m0
                   yes
                   target opp.st0
                   target opp.st1
               ''',
               expect='''
                   opp.spells count 0
                   opp.gy = Call of the Haunted, Magic Jammer
                   you.m0 = Mobius the Frost Monarch atk
               '''),
        cpu('cpu_destroy_human_spell_traps', start=OPP_TURN,
            setup='''
                opp.hand: Mobius the Frost Monarch
                opp.m0: Battle Ox atk
                you.st0: Call of the Haunted set
                you.st1: Magic Jammer set
            ''',
            steps='''
                when prompt response: no
                cpu_turn
            ''',
            expect='''
                you.spells count 0
                opp.monsters has Mobius the Frost Monarch
            '''),
    ],
    'Smashing Ground': [
        player('destroy_highest_def',
               setup='''
                   you.hand: Smashing Ground
                   opp.m0: Celtic Guardian atk
                   opp.m1: Giant Rat atk
               ''',
               steps='activate you.hand[Smashing Ground]',
               expect='''
                   opp.m0 = Celtic Guardian atk
                   opp.m1 = empty
                   opp.gy = Giant Rat
               ''',
               note='Giant Rat has DEF 1450, Celtic Guardian 1200.'),
        cpu('cpu_destroy_highest_def', start=OPP_TURN,
            setup='''
                opp.hand: Smashing Ground
                you.m0: Celtic Guardian atk
                you.m1: Giant Rat atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m1 = empty
                you.gy has Giant Rat
            '''),
    ],
    'Blade Knight': [
        player('boost_and_negate_flip',
               setup='''
                   you.m0: Blade Knight atk
                   you.hand: Fissure
                   opp.m0: Man-Eater Bug set
               ''',
               steps='''
                   check you.m0.atk = 2000
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Man-Eater Bug
                   you.m0 = Blade Knight atk
               ''',
               note="With no other monster of yours, Man-Eater Bug's FLIP effect does not activate."),
        cpu('cpu_negate_flip', start=OPP_TURN,
            setup='''
                opp.m0: Blade Knight atk
                you.m0: Man-Eater Bug set
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                opp.m0 = Blade Knight atk
            '''),
    ],
    'Mystic Swordsman LV2': [
        player('destroy_face_down',
               setup='''
                   you.m0: Mystic Swordsman LV2 atk
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
               ''',
               note="The card text's first effect (YGOPRODeck): it destroys a face-down Defense monster it attacks."),
        player('level_up_declined',
               setup='''
                   you.m0: Mystic Swordsman LV2 atk
                   opp.m0: Kuriboh atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   pass
                   no
               ''',
               expect='''
                   opp.lp = 7400
                   you.m0 = Mystic Swordsman LV2 atk
               ''',
               note='Mystic Swordsman LV4 is not in the ROM, so answer No to the End Phase level-up.'),
        cpu('cpu_destroy_face_down', start=OPP_TURN,
            setup='''
                opp.m0: Mystic Swordsman LV2 atk
                you.m0: Summoned Skull set
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.gy = Summoned Skull
            '''),
    ],
    'Bottomless Trap Hole': [
        player('banish_summoned_monster', start=OPP_TURN,
               setup='''
                   you.st0: Bottomless Trap Hole set
                   opp.hand: Gemini Elf
               ''',
               steps='when summon opp.hand[Gemini Elf]: activate you.st0',
               expect='''
                   opp.banished = Gemini Elf
                   opp.monsters count 0
                   you.gy = Bottomless Trap Hole
               '''),
        cpu('cpu_banish_human_summon',
            setup='''
                opp.st0: Bottomless Trap Hole set
                you.hand: Gemini Elf
            ''',
            steps='summon you.hand[Gemini Elf] -> m0 atk',
            expect='''
                you.m0 = empty
                you.banished = Gemini Elf
                opp.gy = Bottomless Trap Hole
            '''),
    ],
    'Emergency Provisions': [
        player('send_two_gain_2000',
               setup='''
                   you.hand: Emergency Provisions
                   you.st0: Trap Hole set
                   you.st1: Call of the Haunted set
               ''',
               steps='''
                   activate you.hand[Emergency Provisions]
                   pick you.st0
                   pick you.st1
               ''',
               expect='''
                   you.lp = 10000
                   you.gy = Trap Hole, Call of the Haunted, Emergency Provisions
                   you.spells count 0
               '''),
        cpu('cpu_answer_spell_trap_removal', start={'turn': 'you', 'turn_count': 2, 'lp': (8000, 1500)},
            setup='''
                you.hand: Mystical Space Typhoon
                opp.st0: Emergency Provisions set
                opp.st1: Trap Hole set
            ''',
            steps='''
                activate you.hand[Mystical Space Typhoon]
                target opp.st1
            ''',
            expect='''
                opp.lp = 2500
                opp.gy has Emergency Provisions
                opp.gy has Trap Hole
            ''',
            note='With low LP the CPU chains it, sending the targeted Trap Hole for 1000 LP.'),
    ],
    'Enemy Controller': [
        player('change_position',
               setup='''
                   you.hand: Enemy Controller
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Enemy Controller]
                   choose "Change battle position"
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = Summoned Skull def
                   you.gy = Enemy Controller
               '''),
        player('take_control',
               setup='''
                   you.hand: Enemy Controller
                   you.m0: Battle Ox atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Enemy Controller]
                   choose "Take control"
                   target you.m0
                   target opp.m0
                   check you.monsters = Summoned Skull
                   pass
               ''',
               expect='''
                   opp.monsters = Summoned Skull
                   you.gy = Battle Ox, Enemy Controller
               '''),
        cpu('cpu_take_stronger_monster', start=OPP_TURN,
            setup='''
                opp.st0: Enemy Controller set
                opp.m0: Battle Ox atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Enemy Controller
                opp.gy has Battle Ox
                you.monsters = Summoned Skull
            ''',
            note='Control comes back at the End Phase.'),
    ],
    'Mystik Wok': [
        player('tribute_gain_def',
               setup='''
                   you.hand: Mystik Wok
                   you.m0: Giant Rat atk
               ''',
               steps='''
                   activate you.hand[Mystik Wok]
                   target you.m0
                   choose "DEF"
               ''',
               expect='''
                   you.lp = 9450
                   you.gy = Giant Rat, Mystik Wok
               '''),
        cpu('cpu_low_lp_answer_attack', start={'turn': 'you', 'turn_count': 2, 'lp': (8000, 1000)},
            setup='''
                you.m0: Summoned Skull atk
                opp.st0: Mystik Wok set
                opp.m0: Celtic Guardian atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
                when replay: cancel
            ''',
            expect='''
                opp.lp = 2400
                opp.gy has Celtic Guardian
                opp.gy has Mystik Wok
            ''',
            note='It Tributes the attacked Celtic Guardian and takes its ATK (1400, higher than its DEF).'),
    ],
    'Berserk Gorilla': [
        player('destroyed_in_defense',
               setup='you.m0: Berserk Gorilla atk',
               steps='change_position you.m0',
               expect='''
                   you.m0 = empty
                   you.gy = Berserk Gorilla
               '''),
        player('must_attack',
               setup='''
                   you.m0: Berserk Gorilla atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   check menu you.m0 has Change Position
                   battle
                   cancel
                   check you.m0 = Berserk Gorilla atk
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   you.m0 = empty
                   you.lp = 7500
               ''',
               note='B (cancel) cannot end the Battle Phase while the Gorilla can attack.'),
        cpu('cpu_must_attack', start=OPP_TURN,
            setup='''
                opp.m0: Berserk Gorilla atk
                you.m0: Celtic Guardian atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7400
            '''),
    ],
    'Ninja Grandmaster Sasuke': [
        player('destroy_face_up_defense',
               setup='''
                   you.m0: Ninja Grandmaster Sasuke atk
                   opp.m0: Big Shield Gardna def
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Big Shield Gardna
                   you.lp = 8000
               '''),
        cpu('cpu_destroy_face_up_defense', start=OPP_TURN,
            setup='''
                opp.m0: Ninja Grandmaster Sasuke atk
                you.m0: Big Shield Gardna def
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.gy = Big Shield Gardna
            '''),
    ],
    'D.D. Assailant': [
        player('banish_both_when_destroyed',
               setup='''
                   you.m0: D.D. Assailant atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   you.lp = 7200
                   you.banished = D.D. Assailant
                   opp.banished = Summoned Skull
               ''',
               note='No effect row was found for this card: this scenario checks the card text.'),
        cpu('cpu_banish_both_when_destroyed',
            setup='''
                you.m0: Summoned Skull atk
                opp.m0: D.D. Assailant atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                opp.lp = 7200
                you.banished = Summoned Skull
                opp.banished = D.D. Assailant
            '''),
    ],
    'Return from the Different Dimension': [
        player('return_banished_then_banish',
               setup='''
                   you.st0: Return from the Different Dimension set
                   you.banished: Summoned Skull, Battle Ox, Celtic Guardian
               ''',
               steps='''
                   activate you.st0
                   zone m0
                   position atk
                   zone m1
                   position atk
                   zone m2
                   position atk
                   check you.lp = 4000
                   check you.monsters = Summoned Skull, Battle Ox, Celtic Guardian
                   pass
               ''',
               expect='''
                   you.monsters count 0
                   you.banished = Summoned Skull, Battle Ox, Celtic Guardian
               ''',
               note='The strongest ATK comes back first: Summoned Skull, Battle Ox, Celtic Guardian.'),
        cpu('cpu_return_banished', start=OPP_TURN,
            setup='''
                opp.st0: Return from the Different Dimension set
                opp.banished: Summoned Skull, Battle Ox
            ''',
            steps='cpu_turn',
            expect='''
                opp.lp = 4000
                opp.banished = Summoned Skull, Battle Ox
                opp.monsters count 0
            '''),
    ],
    'Monster Gate': [
        player('tribute_excavate_summon',
               setup='''
                   you.hand: Monster Gate
                   you.m0: Celtic Guardian atk
                   you.deck: Fissure, Trap Hole, Summoned Skull
               ''',
               steps='''
                   activate you.hand[Monster Gate]
                   target you.m0
                   zone m0
                   position atk
               ''',
               expect='''
                   you.m0 = Summoned Skull atk
                   you.gy = Monster Gate, Celtic Guardian, Fissure, Trap Hole
               '''),
        cpu('cpu_tribute_excavate_summon', start=OPP_TURN,
            setup='''
                opp.hand: Monster Gate
                opp.m0: Celtic Guardian atk
                opp.deck: Fissure, Summoned Skull
            ''',
            steps='cpu_turn',
            expect='''
                opp.monsters has Summoned Skull
                opp.gy has Fissure
                opp.gy has Monster Gate
            '''),
    ],
    'Monster Reincarnation': [
        player('discard_add_monster',
               setup='''
                   you.hand: Monster Reincarnation, Fissure
                   you.gy: Summoned Skull
               ''',
               steps='''
                   activate you.hand[Monster Reincarnation]
                   discard you.hand[Fissure]
                   pick you.gy[Summoned Skull]
               ''',
               expect='''
                   you.hand = Summoned Skull
                   you.gy = Monster Reincarnation, Fissure
               '''),
        cpu('cpu_add_monster_empty_field', start=OPP_TURN,
            setup='''
                opp.hand: Monster Reincarnation, Fissure
                opp.gy: Summoned Skull
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Monster Reincarnation
                opp.gy lacks Summoned Skull
            '''),
    ],
    "Pikeru's Circle of Enchantment": [
        player('no_effect_damage', start=OPP_TURN,
               setup='''
                   you.st0: Pikeru's Circle of Enchantment set
                   opp.hand: Poison of the Old Man
               ''',
               steps="when activate opp.hand[Poison of the Old Man]: activate you.st0",
               expect='''
                   you.lp = 8000
                   you.gy = Pikeru's Circle of Enchantment
                   opp.gy = Poison of the Old Man
               '''),
        cpu('cpu_chain_to_lethal_burn', start={'turn': 'you', 'turn_count': 2, 'lp': (8000, 800)},
            setup='''
                you.hand: Poison of the Old Man
                opp.st0: Pikeru's Circle of Enchantment set
            ''',
            steps='''
                activate you.hand[Poison of the Old Man]
                choose "Inflict 800 damage"
            ''',
            expect='''
                opp.lp = 800
                opp.gy = Pikeru's Circle of Enchantment
            ''',
            note='The 800 damage is all of its LP, so the CPU chains Pikeru.'),
    ],
    'Mind Control': [
        player('take_control_no_attack',
               setup='''
                   you.hand: Mind Control
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Mind Control]
                   target opp.m0
                   check you.monsters = Summoned Skull
                   battle
                   check cannot attack you.m[Summoned Skull] -> direct
                   pass
               ''',
               expect='''
                   opp.monsters = Summoned Skull
                   opp.lp = 8000
               '''),
        cpu('cpu_take_control', start=OPP_TURN,
            setup='''
                opp.hand: Mind Control
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Mind Control
                you.monsters = Summoned Skull
                you.lp = 8000
            ''',
            note='The CPU takes Summoned Skull for its turn but cannot attack with it; it comes back at the End Phase.'),
    ],
    'Night Assailant': [
        player('flip_destroy',
               setup='''
                   you.m0: Night Assailant set
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   flip_summon you.m0
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Summoned Skull
               '''),
        player('discarded_add_flip_monster',
               setup='''
                   you.hand: Lightning Vortex, Night Assailant
                   you.gy: Man-Eater Bug
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Lightning Vortex]
                   discard you.hand[Night Assailant]
                   pick you.gy[Man-Eater Bug]
               ''',
               expect='''
                   you.hand = Man-Eater Bug
                   you.gy = Night Assailant, Lightning Vortex
                   opp.m0 = empty
               '''),
        cpu('cpu_flipped_by_attack',
            setup='''
                you.m0: Summoned Skull atk
                you.m1: Battle Ox atk
                opp.m0: Night Assailant set
            ''',
            steps='''
                battle
                attack you.m1 -> opp.m0
            ''',
            expect='''
                you.monsters count 1
                opp.m0 = empty
            ''',
            note='Flipped by the attack, its FLIP effect destroys one of your monsters.'),
    ],
    'Dekoichi the Battlechanted Locomotive': [
        player('flip_draw_one',
               setup='''
                   you.m0: Dekoichi the Battlechanted Locomotive set
                   you.deck: Battle Ox
               ''',
               steps='flip_summon you.m0',
               expect='''
                   you.hand = Battle Ox
                   you.m0 = Dekoichi the Battlechanted Locomotive atk
               ''',
               note='Bokoichi the Freightening Car is not in the ROM, so it is always 1 card.'),
        cpu('cpu_flipped_by_attack',
            setup='''
                you.m0: Summoned Skull atk
                opp.m0: Dekoichi the Battlechanted Locomotive set
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                opp.m0 = empty
                opp.hand count 1
            '''),
    ],
    'Lightning Vortex': [
        player('destroy_face_up',
               setup='''
                   you.hand: Lightning Vortex, Fissure
                   opp.m0: Summoned Skull atk
                   opp.m1: Battle Ox atk
                   opp.m2: Man-Eater Bug set
               ''',
               steps='''
                   activate you.hand[Lightning Vortex]
                   discard you.hand[Fissure]
               ''',
               expect='''
                   opp.m0 = empty
                   opp.m1 = empty
                   opp.m2 = Man-Eater Bug set
                   you.gy = Fissure, Lightning Vortex
               '''),
        cpu('cpu_destroy_face_up', start=OPP_TURN,
            setup='''
                opp.hand: Lightning Vortex, Fissure
                you.m0: Summoned Skull atk
                you.m1: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                you.monsters count 0
                opp.gy has Lightning Vortex
            '''),
    ],
    'Phoenix Wing Wind Blast': [
        player('return_to_deck_top',
               setup='''
                   you.st0: Phoenix Wing Wind Blast set
                   you.hand: Fissure
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.st0
                   discard you.hand[Fissure]
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.deck[0] = Summoned Skull
                   you.gy = Fissure, Phoenix Wing Wind Blast
               '''),
        cpu('cpu_return_to_deck_top', start=OPP_TURN,
            setup='''
                opp.st0: Phoenix Wing Wind Blast set
                opp.hand: Fissure
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.deck[0] = Summoned Skull
            '''),
    ],
    'Xing Zhen Hu': [
        player('lock_two_set_cards',
               setup='''
                   you.st0: Xing Zhen Hu set
                   you.hand: Gemini Elf
                   opp.st0: Trap Hole set
                   opp.st1: Call of the Haunted set
               ''',
               steps='''
                   activate you.st0
                   target opp.st0
                   target opp.st1
                   pass
                   cpu_turn
                   draw
                   summon you.hand[Gemini Elf] -> m0 atk
               ''',
               expect='''
                   you.m0 = Gemini Elf atk
                   opp.st0 = Trap Hole set
                   you.st0 = Xing Zhen Hu up
               ''',
               note='The locked Trap Hole cannot answer your summon (the opponent is not frozen).'),
        cpu('cpu_lock_human_set_cards', start=OPP_TURN,
            setup='''
                opp.st0: Xing Zhen Hu set
                you.st0: Trap Hole set
                you.st1: Call of the Haunted set
            ''',
            steps='''
                cpu_turn
                draw
            ''',
            expect='''
                opp.st0 = Xing Zhen Hu up
                menu you.st0 lacks Activate
                menu you.st1 lacks Activate
            '''),
    ],
    'Brain Control': [
        player('pay_800_take_control',
               setup='''
                   you.hand: Brain Control
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.hand[Brain Control]
                   target opp.m0
                   check you.lp = 7200
                   battle
                   attack you.m[Summoned Skull] -> direct
                   pass
               ''',
               expect='''
                   opp.lp = 5500
                   opp.monsters = Summoned Skull
               '''),
        cpu('cpu_pay_800_take_control', start=OPP_TURN,
            setup='''
                opp.hand: Brain Control
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.lp = 7200
                you.lp = 5500
                you.monsters = Summoned Skull
            '''),
    ],
    'Zaborg the Thunder Monarch': [
        player('tribute_summon_destroy',
               setup='''
                   you.hand: Zaborg the Thunder Monarch
                   you.m0: Battle Ox atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   tribute_summon you.hand[Zaborg the Thunder Monarch] -> m0 atk tributes you.m0
                   target opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.gy = Summoned Skull
                   you.m0 = Zaborg the Thunder Monarch atk
               '''),
        cpu('cpu_tribute_summon_destroy', start=OPP_TURN,
            setup='''
                opp.hand: Zaborg the Thunder Monarch
                opp.m0: Battle Ox atk
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                opp.monsters has Zaborg the Thunder Monarch
            '''),
    ],
    'Level Limit - Area B': [
        player('level_4_to_defense',
               setup='''
                   you.hand: Level Limit - Area B
                   you.m0: Battle Ox atk
                   opp.m0: Summoned Skull atk
                   opp.m1: Man-Eater Bug atk
               ''',
               steps='activate you.hand[Level Limit - Area B]',
               expect='''
                   you.m0 = Battle Ox def
                   opp.m0 = Summoned Skull def
                   opp.m1 = Man-Eater Bug atk
                   menu you.m0 lacks Change Position
               '''),
        cpu('cpu_level_4_to_defense', start=OPP_TURN,
            setup='''
                opp.hand: Level Limit - Area B
                you.m0: Summoned Skull atk
            ''',
            steps='cpu_turn',
            expect='you.m0 = Summoned Skull def'),
    ],
}
