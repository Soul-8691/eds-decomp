"""Exact scenarios (tools/testing_scenarios/exact.py): Cyber and Reaper."""
from exact import cpu, player

OPP_TURN = {'turn': 'opp', 'turn_count': 3}

SCENARIOS = {
    'Cyber Twin Dragon': [
        player('second_attack',
               setup='''
                   you.m0: Cyber Twin Dragon atk
                   opp.m0: Celtic Guardian atk
                   opp.m1: Battle Ox atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   attack you.m0 -> opp.m1
               ''',
               expect='''
                   opp.monsters count 0
                   opp.lp = 5500
               '''),
        cpu('cpu_second_attack', start=OPP_TURN,
            setup='''
                opp.m0: Cyber Twin Dragon atk
                you.m0: Celtic Guardian atk
                you.m1: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                you.monsters count 0
                you.lp = 5500
            '''),
    ],
    'Cyber Dragon': [
        player('special_summon_from_hand',
               setup='''
                   you.hand: Cyber Dragon
                   opp.m0: Summoned Skull atk
               ''',
               steps='special_summon you.hand[Cyber Dragon] -> m0 atk',
               expect='you.m0 = Cyber Dragon atk'),
        player('not_while_you_control_a_monster',
               setup='''
                   you.hand: Cyber Dragon
                   you.m0: Battle Ox atk
                   opp.m0: Summoned Skull atk
               ''',
               steps='check menu you.hand[Cyber Dragon] lacks Special Summon',
               expect='you.hand = Cyber Dragon'),
        cpu('cpu_special_summon_first', start=OPP_TURN,
            setup='''
                opp.hand: Cyber Dragon
                you.m0: Battle Ox atk
            ''',
            steps='cpu_turn',
            expect='''
                opp.monsters has Cyber Dragon
                you.m0 = empty
                you.lp = 7600
            '''),
    ],
    'Cyber End Dragon': [
        player('pierce',
               setup='''
                   you.m0: Cyber End Dragon atk
                   opp.m0: Big Shield Gardna def
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
               ''',
               expect='''
                   opp.m0 = empty
                   opp.lp = 6600
               '''),
        cpu('cpu_pierce', start=OPP_TURN,
            setup='''
                opp.m0: Cyber End Dragon atk
                you.m0: Big Shield Gardna def
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 6600
            '''),
    ],
    'Threatening Roar': [
        player('stop_further_attacks', start=OPP_TURN,
               setup='''
                   you.st0: Threatening Roar set
                   opp.m0: Summoned Skull atk
                   opp.m1: Battle Ox atk
               ''',
               steps='when attack opp.m0 -> direct: activate you.st0',
               expect='''
                   you.lp in 5500|6300
                   you.gy = Threatening Roar
               ''',
               note='The declared attack goes on (2500); Battle Ox cannot attack after it. If Battle Ox attacks '
                    'first, answer that attack instead (then you.lp is 6300).'),
        cpu('cpu_before_battle_phase',
            setup='''
                you.m0: Summoned Skull atk
                opp.st0: Threatening Roar set
            ''',
            steps='''
                battle
                check cannot attack you.m0 -> direct
            ''',
            expect='''
                opp.lp = 8000
                opp.gy = Threatening Roar
            ''',
            note='The CPU flips it as your Battle Phase starts, since you have a monster with 2000+ ATK.'),
    ],
    'Exarion Universe': [
        player('lose_400_to_pierce',
               setup='''
                   you.m0: Exarion Universe atk
                   opp.m0: Celtic Guardian def
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   yes
               ''',
               expect='''
                   opp.m0 = empty
                   opp.lp = 7800
                   you.m0.atk = 1400
               '''),
        cpu('cpu_lose_400_to_pierce', start=OPP_TURN,
            setup='''
                opp.m0: Exarion Universe atk
                you.m0: Celtic Guardian def
            ''',
            steps='cpu_turn',
            expect='''
                you.m0 = empty
                you.lp = 7800
            '''),
    ],
    'Gravity Bind': [
        player('stop_level_4_attacks',
               setup='''
                   you.st0: Gravity Bind set
                   opp.m0: Summoned Skull atk
               ''',
               steps='''
                   activate you.st0
                   pass
                   cpu_turn
               ''',
               expect='''
                   you.lp = 8000
                   you.st0 = Gravity Bind up
               '''),
        cpu('cpu_stop_human_attacks', start=OPP_TURN,
            setup='''
                opp.st0: Gravity Bind set
                you.m0: Summoned Skull atk
            ''',
            steps='''
                cpu_turn
                draw
                battle
            ''',
            expect='''
                opp.st0 = Gravity Bind up
                cannot attack you.m0 -> direct
                opp.lp = 8000
            '''),
    ],
    'Chiron the Mage': [
        player('discard_spell_destroy',
               setup='''
                   you.m0: Chiron the Mage atk
                   you.hand: Fissure
                   opp.st0: Trap Hole set
               ''',
               steps='''
                   activate you.m0
                   discard you.hand[Fissure]
                   target opp.st0
               ''',
               expect='''
                   opp.st0 = empty
                   opp.gy = Trap Hole
                   you.gy = Fissure
                   menu you.m0 lacks Activate
               '''),
        cpu('cpu_discard_spell_destroy', start=OPP_TURN,
            setup='''
                opp.m0: Chiron the Mage atk
                opp.hand: Fissure
                you.st0: Trap Hole set
            ''',
            steps='cpu_turn',
            expect='''
                you.st0 = empty
                you.gy = Trap Hole
            '''),
    ],
    'Cursed Seal of the Forbidden Spell': [
        player('negate_spell', start=OPP_TURN,
               setup='''
                   you.st0: Cursed Seal of the Forbidden Spell set
                   you.hand: Fissure
                   opp.hand: Pot of Greed
               ''',
               steps='''
                   when activate opp.hand[Pot of Greed]: activate you.st0
                   discard you.hand[Fissure]
               ''',
               expect='''
                   opp.gy = Pot of Greed
                   opp.hand count 0
                   you.gy = Cursed Seal of the Forbidden Spell, Fissure
               '''),
        cpu('cpu_negate_human_spell',
            setup='''
                you.hand: Pot of Greed
                opp.st0: Cursed Seal of the Forbidden Spell set
                opp.hand: Fissure
            ''',
            steps='activate you.hand[Pot of Greed]',
            expect='''
                you.hand count 0
                you.gy = Pot of Greed
                opp.gy = Cursed Seal of the Forbidden Spell, Fissure
            '''),
    ],
    'Wave-Motion Cannon': [
        player('burn_after_one_standby',
               setup='you.hand: Wave-Motion Cannon',
               steps='''
                   activate you.hand[Wave-Motion Cannon]
                   pass
                   opp_turn
                   draw
                   activate you.st[Wave-Motion Cannon]
               ''',
               expect='''
                   opp.lp = 7000
                   you.gy = Wave-Motion Cannon
               ''',
               note='One of your Standby Phases has passed since it was activated: 1000 damage.'),
        cpu('cpu_activate_and_keep', start=OPP_TURN,
            setup='opp.hand: Wave-Motion Cannon',
            steps='cpu_turn',
            expect='opp.spells has Wave-Motion Cannon'),
    ],
    'Treeborn Frog': [
        player('revive_in_standby',
               setup='you.gy: Treeborn Frog',
               steps='''
                   pass
                   opp_turn
                   draw
                   yes
                   position atk
               ''',
               expect='''
                   you.monsters = Treeborn Frog
                   you.gy count 0
               '''),
        cpu('cpu_revive_in_standby',
            setup='opp.gy: Treeborn Frog',
            steps='''
                pass
                cpu_turn
            ''',
            expect='opp.gy lacks Treeborn Frog'),
    ],
    'Thestalos the Firestorm Monarch': [
        player('discard_monster_burn',
               setup='''
                   you.hand: Thestalos the Firestorm Monarch
                   you.m0: Battle Ox atk
                   opp.hand: Summoned Skull
               ''',
               steps='tribute_summon you.hand[Thestalos the Firestorm Monarch] -> m0 atk tributes you.m0',
               expect='''
                   opp.hand count 0
                   opp.gy = Summoned Skull
                   opp.lp = 7400
               ''',
               note='Summoned Skull is the only card in hand, so the random discard is known: Level 6, 600 damage.'),
        cpu('cpu_discard_monster_burn', start=OPP_TURN,
            setup='''
                opp.hand: Thestalos the Firestorm Monarch
                opp.m0: Battle Ox atk
                you.hand: Summoned Skull
            ''',
            steps='cpu_turn',
            expect='''
                you.hand count 0
                you.gy = Summoned Skull
                you.lp <= 7400
            '''),
    ],
    'D.D. Survivor': [
        player('return_at_end_phase',
               setup='''
                   you.m0: D.D. Survivor atk
                   opp.m0: D.D. Warrior Lady atk
               ''',
               steps='''
                   battle
                   attack you.m0 -> opp.m0
                   check you.banished = D.D. Survivor
                   pass
                   position atk
               ''',
               expect='''
                   you.monsters = D.D. Survivor
                   you.banished count 0
                   opp.banished = D.D. Warrior Lady
               ''',
               note="D.D. Warrior Lady's controller always banishes both (the CPU says Yes)."),
        cpu('cpu_return_at_end_phase',
            setup='''
                you.m0: D.D. Warrior Lady atk
                opp.m0: D.D. Survivor atk
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
                yes
                pass
            ''',
            expect='''
                opp.monsters = D.D. Survivor
                you.banished = D.D. Warrior Lady
            '''),
    ],
    'Pot of Avarice': [
        player('shuffle_five_draw_two',
               setup='''
                   you.hand: Pot of Avarice
                   you.gy: Battle Ox, Celtic Guardian, La Jinn the Mystical Genie of the Lamp, Summoned Skull, Man-Eater Bug
               ''',
               steps='''
                   activate you.hand[Pot of Avarice]
                   pick you.gy[Battle Ox]
                   pick you.gy[Celtic Guardian]
                   pick you.gy[La Jinn the Mystical Genie of the Lamp]
                   pick you.gy[Summoned Skull]
                   pick you.gy[Man-Eater Bug]
               ''',
               expect='''
                   you.gy = Pot of Avarice
                   you.hand count 2
               '''),
        cpu('cpu_shuffle_five_draw_two', start=OPP_TURN,
            setup='''
                opp.hand: Pot of Avarice
                opp.gy: Battle Ox, Celtic Guardian, La Jinn the Mystical Genie of the Lamp, Summoned Skull, Man-Eater Bug
            ''',
            steps='cpu_turn',
            expect='''
                opp.gy has Pot of Avarice
                opp.gy lacks Summoned Skull
            '''),
    ],
    'Magical Merchant': [
        player('flip_excavate_spell_trap',
               setup='''
                   you.m0: Magical Merchant set
                   you.deck: Battle Ox, Celtic Guardian, Fissure, Trap Hole
               ''',
               steps='flip_summon you.m0',
               expect='''
                   you.hand = Fissure
                   you.gy = Battle Ox, Celtic Guardian
                   you.deck[0] = Trap Hole
               '''),
        cpu('cpu_flipped_by_attack',
            setup='''
                you.m0: Summoned Skull atk
                opp.m0: Magical Merchant set
                opp.deck: Battle Ox, Fissure
            ''',
            steps='''
                battle
                attack you.m0 -> opp.m0
            ''',
            expect='''
                opp.hand = Fissure
                opp.gy has Battle Ox
                opp.gy has Magical Merchant
            '''),
    ],
    'Mask of Restrict': [
        player('no_tributes',
               setup='''
                   you.st0: Mask of Restrict set
                   you.m0: Battle Ox atk
                   you.hand: Summoned Skull
               ''',
               steps='activate you.st0',
               expect='''
                   menu you.hand[Summoned Skull] lacks Summon
                   you.st0 = Mask of Restrict up
               '''),
        cpu('cpu_lock_tributes', start=OPP_TURN,
            setup='''
                opp.st0: Mask of Restrict set
                you.m0: Battle Ox atk
                you.hand: Summoned Skull
            ''',
            steps='''
                cpu_turn
                draw
            ''',
            expect='''
                opp.st0 = Mask of Restrict up
                menu you.hand[Summoned Skull] lacks Summon
            '''),
    ],
}
