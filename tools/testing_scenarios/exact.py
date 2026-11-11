"""Exact TESTING.md scenarios: the format, its parser, and the Markdown and Lua it becomes.

The scenarios themselves are in exact_cards_*.py (one `SCENARIOS` dict each: card name -> list of scenarios). Every
scenario is reproducible as written: the whole starting state, every input, and the state to check after. Card
names are the ROM's (gCardNames). tools/gen_testing_md.py renders them into TESTING.md (and checks every card named
is legal in the card's format); tools/gen_testing_lua.py turns them into the mGBA scripts' data.

A scenario is `player(...)` (the human plays the card; the CPU is frozen unless a step needs it) or `cpu(...)` (the
CPU's turn or response is under test):

  effect   a short snake_case name: the recording / script name is <Card>_<Effect>_<Player|CPU>.
  start    where the setup is applied, in the opening of that turn's Main Phase 1 (both hands are replaced by the
           setup, the Draw Phase already done): {'turn': 'you'|'opp', 'turn_count': N (gDuel.turnCount; 0 is the
           duel's first turn, which has no Battle Phase), 'lp': (you, opp)}. DEFAULT_START otherwise.
  setup    one line per place, `<side>.<place>: <cards>`, side `you` (player 0) or `opp` (player 1):
             hand          cards in hand order (slot 0 first)
             m0 .. m4      a Monster Zone (gDuelPlayers[p] zone 0-4): `<card> atk|def|set` (atk/def face up, set
                           face-down Defense); `ss` after it marks it Special Summoned (ZONE_STATUS_SPECIAL_SUMMONED)
             st0 .. st4    a Spell/Trap Zone (zones 5-9): `<card> set|up` (Set, or face up)
             fz            the Field Zone: `<card> up`
             deck          the top of the Deck, top card first (deck[0] is the next draw); the rest of the Deck is
                           the format's usage deck, as the scripts load it
             gy, banished, extra (the Fusion Deck)   the whole pile, in that order
           Anything not listed is empty (the Deck excepted), but in a CPU scenario the CPU's board and piles from its
           first turn stay (only its hand and the listed places are replaced), and while it plays the CPU Sets at
           most 4 Spell/Traps (a zone stays free for its Normal Spell). LP and the turn are in `start`.
  steps    the inputs, in order (STEP_VERBS below); `when <event>: <step>` waits for that event first.
  expect   the state after the last step (EXPECT_FORMS below). `check <expect>` in the steps checks it mid-way.
  note     optional: what the scenario shows, or why an expectation is as it is.

Places in steps and expectations: `you.hand[Card Name]` (that card in the hand), `you.hand[0]` (slot 0),
`you.m0` (Monster Zone 0), `opp.m[Card Name]` (the monster of that name on that side), `you.st1`, `you.fz`,
`you.gy[Card Name]`, `you.deck[Card Name]`, `you.extra[Card Name]`, `you.banished[Card Name]`; and in
expectations the whole piles `you.hand`, `you.gy`, `you.banished`, `you.extra`, `you.monsters` (zones 0-4),
`you.spells` (zones 5-10).
"""
from __future__ import annotations

import re

DEFAULT_START = {'turn': 'you', 'turn_count': 2, 'lp': (8000, 8000)}

SIDES = ('you', 'opp')
PILES = ('hand', 'deck', 'gy', 'banished', 'extra')
ZONE_RE = r'(?:m[0-4]|st[0-4]|fz)'
P = r'((?:you|opp)\.[a-z0-9]+(?:\[[^\]]+\])?)'     # a place argument (card names may hold spaces)
PLACE_RE = re.compile(r'^(you|opp)\.(hand|deck|gy|banished|extra|monsters|spells|m[0-4]|st[0-4]|fz|m|st)'
                      r'(?:\[(.+?)\])?$')
POSITIONS = {'atk': 'face-up Attack Position', 'def': 'face-up Defense Position', 'set': 'face-down (Set)',
             'up': 'face up'}

# verb: (argument pattern, description template). {0}, {1}... are the regex groups.
STEP_VERBS = {
    'activate': (P + r'', 'Open the card menu of {0} and choose Activate.'),
    'summon': (P + r' -> ({z}) (atk|def)', 'Normal Summon {0} into {1} ({2}).'),
    'set': (P + r' -> ({z})', 'Set {0} into {1}.'),
    'tribute_summon': (P + r' -> ({z}) (atk|def) tributes (.+)',
                       'Tribute Summon {0} into {1} ({2}), Tributing {3}.'),
    'special_summon': (P + r' -> ({z}) (atk|def)(?: banish (.+))?',
                       'Special Summon {0} from the hand by its own rule into {1} ({2}){3}.'),
    'fusion_summon': (r'(.+?) from (.+) -> ({z}) (atk|def)',
                      'Activate Polymerization from your hand: Fusion Summon {0} from {1} into {2} ({3}).'),
    'flip_summon': (P + r'', 'Flip Summon {0}.'),
    'change_position': (P + r'', 'Change the battle position of {0} (card menu).'),
    'target': (P + r'', 'Pick {0} as the target.'),
    'pick': (P + r'', 'Pick {0} in the list or on the field.'),
    'discard': (P + r'', 'Discard {0} for the cost.'),
    'yes': (r'', 'Answer Yes.'),
    'no': (r'', 'Answer No.'),
    'choose': (r'"(.+)"', 'Choose "{0}" in the menu.'),
    'declare': (r'(.+)', 'Declare {0}.'),
    'position': (r'(atk|def)', 'Choose {0} for the summoned monster.'),
    'zone': (r'({z})', 'Choose {0} for the summoned monster.'),
    'battle': (r'', 'Enter the Battle Phase.'),
    'attack': (P + r' -> ' + r'(' + P[1:-1] + r'|direct)', 'Attack with {0}: target {1}.'),
    'cancel': (r'', 'Cancel (B): do not attack again / make no choice.'),
    'main2': (r'', 'Go to Main Phase 2.'),
    'pass': (r'', 'End your turn (its End Phase effects resolve).'),
    'opp_turn': (r'', "The opponent's turn passes: frozen, it plays nothing (its own Draw Phase and phase effects "
                      "still run)."),
    'cpu_turn': (r'', "The CPU plays its whole turn (not frozen)."),
    'draw': (r'', 'In your Draw Phase, open your Main Deck and choose Draw; stop at Main Phase 1.'),
    'check': (r'(.+)', 'Check: {0}.'),
    'note': (r'(.+)', '{0}'),
}
WHEN_EVENTS = {
    'attack': r'attack ' + P + r' -> (' + P[1:-1] + r'|direct)',
    'activate': r'activate ' + P,
    'summon': r'summon ' + P,
    'prompt': r'prompt (.+)',
    'replay': r'replay',
}
EXPECT_FORMS = [
    (P + r' = empty', '{0} is empty.'),
    (P + r' = (.+?) (atk|def|set|up)', '{0} holds {1}, {2}.'),
    (P + r'\.(atk|def) = (\d+)', 'The {1} of {0} is {2}.'),
    (r'(you|opp)\.lp (=|<=|>=|<|>) (\d+)', 'LP of {0} {1} {2}.'),
    (r'(you|opp)\.lp in ([\d|]+)', 'LP of {0} is one of {1}.'),
    (P + r' count (\d+)', '{0} holds {1} card(s).'),
    (P + r' has (.+)', '{0} includes {1}.'),
    (P + r' lacks (.+)', '{0} does not include {1}.'),
    (P + r' = (.+)', '{0} is exactly {1}.'),
    (r'menu ' + P + r' (has|lacks) (.+)', 'The card menu of {0} {1} {2}.'),
    (r'cannot attack ' + P + r' -> (' + P[1:-1] + r'|direct)', '{0} cannot attack {1}.'),
    (r'battle_phase blocked', 'The Battle Phase cannot be entered.'),
    (r'note (.+)', '{0}'),
]


# `video` (optional): what the recording must show, when the card's chain or battle is not enough (lib_steps.lua):
#   start='summon'   the card's own summon is its effect (The Fiend Megacyber's Special Summon from the hand)
#   start='first'    the capture starts at the first cursor move (the summon that lets a Trap go off), not at the
#                    activation
#   until='defense'  after the effect, wait until the card is in Defense Position (Goblin Attack Force)
#   show='opp.m0'    after the effect, put the cursor on that zone for a moment: the info bar shows its ATK
#   effect='none'    nothing is meant to happen (a card that cannot be used): no wait for an effect
#   clear='cpu_monsters'  a CPU scenario empties the CPU's monster zones too (it keeps its first-turn board
#                    otherwise): with a monster of its own it Tribute Summons instead of using Soul Exchange or
#                    The Fiend Megacyber's Special Summon
#   chain='show'     keep the chain window ("Chain: Activating / Resolving"; hidden by default for cleaner videos)
#   cards='show'     keep the card-showing animations (hidden by default)
#   prompts='show'   draw the prompts (by default they work, answered by the steps, but are not drawn)
#   lists='show'     keep the card lists (choose cards from the Deck, Graveyard...); left out of the video by default
#   battle='show'    keep the battle animations (the attack's sword and banner, the full-screen battle scene)
VIDEO_KEYS = {'start': ('summon', 'first'), 'until': ('defense',), 'show': None, 'effect': ('none',),
              'clear': ('cpu_monsters',), 'chain': ('show',), 'cards': ('show',), 'prompts': ('show',),
              'battle': ('show',), 'lists': ('show',)}


def player(effect, *, setup, steps, expect, start=None, note=None, video=None):
    return _scenario('Player', effect, setup, steps, expect, start, note, video)


def cpu(effect, *, setup, steps, expect, start=None, note=None, video=None):
    return _scenario('CPU', effect, setup, steps, expect, start, note, video)


def _scenario(actor, effect, setup, steps, expect, start, note, video=None):
    st = dict(DEFAULT_START)
    st.update(start or {})
    video = dict(video or {})
    for k, v in video.items():
        allowed = VIDEO_KEYS.get(k, ())
        if k not in VIDEO_KEYS or (allowed is not None and v not in allowed):
            raise ValueError('scenario %s: video %s=%r is not one of %s' % (effect, k, v, VIDEO_KEYS))
    return {'actor': actor, 'effect': effect, 'start': st, 'setup': _lines(setup), 'steps': _lines(steps),
            'expect': _lines(expect), 'note': note, 'video': video}


def _lines(text):
    if isinstance(text, (list, tuple)):
        return [t.strip() for t in text if t.strip()]
    return [t.strip() for t in text.strip().splitlines() if t.strip() and not t.strip().startswith('#')]


# ---- parsing ----

def parse_setup(lines):
    """[(side, place, [(card, position, special)])] with errors raised as ValueError."""
    out = []
    for line in lines:
        m = re.fullmatch(r'(you|opp)\.(hand|deck|gy|banished|extra|m[0-4]|st[0-4]|fz):\s*(.+)', line)
        if not m:
            raise ValueError(f'setup line not understood: {line!r}')
        side, place, rest = m.groups()
        cards = []
        if place in PILES:
            cards = [(c.strip(), None, False) for c in rest.split(',')]
        else:
            mm = re.fullmatch(r'(.+?) (atk|def|set|up)( ss)?', rest.strip())
            if not mm:
                raise ValueError(f'setup zone needs a position: {line!r}')
            pos = mm.group(2)
            if place.startswith('m') and pos == 'up' or not place.startswith('m') and pos in ('atk', 'def'):
                raise ValueError(f'position {pos!r} does not fit {place}: {line!r}')
            cards = [(mm.group(1), pos, bool(mm.group(3)))]
        out.append((side, place, cards))
    return out


def setup_cards(setup):
    """{(side, place): [card names]} of a scenario's setup."""
    return {(s, p): [c for c, _, _ in cards] for s, p, cards in parse_setup(setup)}


def place_name(ref, setup):
    """A place in words, with the card it names or holds."""
    m = PLACE_RE.match(ref)
    if not m:
        return f'`{ref}`'
    side, place, key = m.groups()
    who = 'your' if side == 'you' else "the opponent's"
    cards = setup_cards(setup)
    names = {'hand': 'hand', 'deck': 'Deck', 'gy': 'Graveyard', 'banished': 'banished cards', 'extra': 'Fusion Deck',
             'monsters': 'monsters', 'spells': 'Spells/Traps', 'fz': 'Field Zone', 'm': 'monster',
             'st': 'Spell/Trap'}
    if place[0] in 'ms' and place[-1].isdigit():
        kind = 'Monster Zone' if place[0] == 'm' else 'Spell/Trap Zone'
        held = cards.get((side, place))
        return f'{who} {kind} {place[-1]}' + (f' (**{held[0]}**)' if held else '')
    if key is None:
        return f'{who} {names[place]}'
    if key.isdigit():
        held = cards.get((side, place)) or []
        k = int(key)
        return f'{who} {names[place]} slot {k}' + (f' (**{held[k]}**)' if k < len(held) else '')
    return f'**{key}** in {who} {names[place]}' if place not in ('m', 'st') else f'{who} **{key}**'


def describe_step(step, setup):
    """(code, words) of a step; raises ValueError for an unknown one."""
    if step.startswith('when '):
        m = re.fullmatch(r'when (.+?): (.+)', step)     # (the event holds no ': ')
        if not m:
            raise ValueError(f'when-step not understood: {step!r}')
        event, action = m.groups()
        if not any(re.fullmatch(p, event) for p in WHEN_EVENTS.values()):
            raise ValueError(f'unknown when-event: {event!r}')
        _, words = describe_step(action, setup)
        return step, f'When {_event_words(event, setup)}: {words[0].lower()}{words[1:]}'
    verb, _, args = step.partition(' ')
    if verb not in STEP_VERBS:
        raise ValueError(f'unknown step verb: {step!r}')
    pattern, template = STEP_VERBS[verb]
    m = re.fullmatch(pattern.replace('{z}', ZONE_RE), args)
    if m is None:
        raise ValueError(f'step arguments not understood: {step!r}')
    groups = [g or '' for g in m.groups()]
    if verb == 'check':
        _check_expect(groups[0])
        return step, f'Check: {describe_expect(groups[0], setup)}'
    if verb == 'activate' and re.match(r'(?:you|opp)\.m', groups[0]):
        return step, f'Open the card menu of {place_name(groups[0], setup)} and activate its effect.'
    words = []
    for i, g in enumerate(groups):
        if verb == 'special_summon' and i == 3:
            words.append(', banishing ' + ' and '.join(place_name(x.strip(), setup) for x in g.split(',')) if g else '')
        elif verb == 'tribute_summon' and i == 3:
            words.append(' and '.join(place_name(x.strip(), setup) for x in g.split(',')))
        elif verb == 'fusion_summon' and i == 1:
            words.append(' and '.join(place_name(x.strip(), setup) for x in g.split(',')))
        elif g in POSITIONS:
            words.append(POSITIONS[g])
        elif re.fullmatch(ZONE_RE, g):
            words.append(place_name('you.' + g, setup))     # (the human's own zone: summons are the human's)
        elif g == 'direct':
            words.append('the opponent directly')
        elif PLACE_RE.match(g):
            words.append(place_name(g, setup))
        else:
            words.append(g if verb not in ('fusion_summon',) else f'**{g}**')
    return step, template.format(*words)


def _event_words(event, setup):
    m = re.fullmatch(WHEN_EVENTS['attack'], event)
    if m:
        return f'{place_name(m.group(1), setup)} declares an attack on ' + (
            'you directly' if m.group(2) == 'direct' else place_name(m.group(2), setup))
    m = re.fullmatch(WHEN_EVENTS['activate'], event)
    if m:
        return f'{place_name(m.group(1), setup)} is activated'
    m = re.fullmatch(WHEN_EVENTS['summon'], event)
    if m:
        return f'{place_name(m.group(1), setup)} is summoned'
    m = re.fullmatch(WHEN_EVENTS['prompt'], event)
    if m:
        return f'the {m.group(1)} prompt opens'
    return 'the attack is replayed (its target left the field)'


def _check_expect(text):
    if not any(re.fullmatch(p, text) for p, _ in EXPECT_FORMS):
        raise ValueError(f'expectation not understood: {text!r}')


def describe_expect(text, setup):
    for pattern, template in EXPECT_FORMS:
        m = re.fullmatch(pattern, text)
        if m:
            words = []
            for g in m.groups():
                if g == 'direct':
                    words.append('the opponent directly')
                elif g and PLACE_RE.match(g):
                    words.append(place_name(g, setup))
                elif g in POSITIONS:
                    words.append(POSITIONS[g])
                elif g in ('has', 'lacks') and pattern.startswith('menu'):
                    words.append('offers' if g == 'has' else 'does not offer')
                else:
                    words.append(g)
            return template.format(*words)
    raise ValueError(f'expectation not understood: {text!r}')


def frozen(sc):
    """1 when the opponent stays frozen the whole scenario (no CPU turn, response or choice is part of it)."""
    if sc['actor'] == 'CPU' or sc['start']['turn'] == 'opp':
        return False
    return not any(s.startswith(('cpu_turn', 'when ')) for s in sc['steps'])


def card_names(sc):
    """Every card name a scenario uses (setup, steps and expectations)."""
    names = set()
    for _, _, cards in parse_setup(sc['setup']):
        names.update(c for c, _, _ in cards)
    for line in sc['steps'] + sc['expect']:
        names.update(re.findall(r'\[([^\]\d][^\]]*)\]', line))
        m = re.search(r'fusion_summon (.+?) from ', line)
        if m:
            names.add(m.group(1))
        m = re.fullmatch(r'(?:check )?\S+ (?:=|has|lacks) (.+?)(?: (?:atk|def|set|up))?', line)
        if m and not re.fullmatch(r'empty|\d+|[\d|]+', m.group(1)) and not line.startswith(('menu', 'check menu')):
            names.update(n.strip() for n in m.group(1).split(','))
    return names


def validate(card, sc):
    """Raises ValueError for anything in the scenario the format does not allow."""
    parse_setup(sc['setup'])
    for step in sc['steps']:
        describe_step(step, sc['setup'])
    for e in sc['expect']:
        _check_expect(e)
    if sc['start']['turn'] not in SIDES:
        raise ValueError(f'{card}: start turn must be you or opp')
    if not re.fullmatch(r'[a-z0-9_]+', sc['effect']):
        raise ValueError(f'{card}: effect must be snake_case: {sc["effect"]!r}')


# ---- output ----

def markdown(card, index, sc):
    """Markdown lines for one scenario."""
    st = sc['start']
    who = 'your' if st['turn'] == 'you' else "the opponent's"
    title = sc['effect'].removeprefix('cpu_').replace('_', ' ').capitalize()
    lines = [f'#### {sc["actor"]} {index}: {title}', '',
             f'`{basename(card, sc)}` · start: {who} turn, Main Phase 1 (turnCount {st["turn_count"]}) · '
             f'LP {st["lp"][0]} / {st["lp"][1]} · opponent {"frozen" if frozen(sc) else "not frozen"}', '']
    if sc.get('note'):
        lines += [sc['note'], '']
    lines += ['| Side | Place | Card | Position |', '|---|---|---|---|']
    for side, place, cards in parse_setup(sc['setup']):
        label = {'hand': 'Hand', 'deck': 'Deck', 'gy': 'Graveyard', 'banished': 'Banished',
                 'extra': 'Fusion Deck', 'fz': 'Field Zone'}.get(place)
        if label is None:
            label = ('Monster Zone ' if place[0] == 'm' else 'Spell/Trap Zone ') + place[-1]
        for k, (c, pos, ss) in enumerate(cards):
            slot = f' {k}' if place in ('hand', 'deck') else ''
            p = POSITIONS.get(pos, '') + (', Special Summoned' if ss else '')
            lines.append(f'| {"You" if side == "you" else "Opponent"} | {label}{slot} | {c} | {p} |')
    lines += ['', '**Steps**', '']
    for i, step in enumerate(sc['steps'], 1):
        code, words = describe_step(step, sc['setup'])
        lines.append(f'{i}. `{code}`: {words}')
    lines += ['', '**Expected**', '']
    for e in sc['expect']:
        lines.append(f'- `{e}`: {describe_expect(e, sc["setup"])}')
    lines.append('')
    return lines


def _slug(s):
    s = re.sub(r'[^\w\s-]', '', s)
    s = re.sub(r'[-\s]+', '_', s.strip())
    return (s[:56] if s else 'effect').strip('_')


def basename(card, sc):
    effect = '_'.join(p.capitalize() for p in sc['effect'].split('_') if p)[:56]
    return f'{_slug(card)}_{effect}_{sc["actor"]}'


LUA_KEYWORDS = {'and', 'break', 'do', 'else', 'elseif', 'end', 'false', 'for', 'function', 'goto', 'if', 'in',
                'local', 'nil', 'not', 'or', 'repeat', 'return', 'then', 'true', 'until', 'while'}


def lua_value(v):
    if isinstance(v, bool):
        return 'true' if v else 'false'
    if isinstance(v, (int, float)):
        return str(v)
    if isinstance(v, str):
        return '"' + v.replace('\\', '\\\\').replace('"', '\\"') + '"'
    if isinstance(v, dict):
        # (a key that is a Lua keyword, such as `until`, or not a plain name is written ["key"])
        def key(k):
            return k if re.match(r'^[A-Za-z_]\w*$', k) and k not in LUA_KEYWORDS else '[' + lua_value(k) + ']'
        return '{ ' + ', '.join(f'{key(k)} = {lua_value(x)}' for k, x in v.items()) + ' }'
    return '{ ' + ', '.join(lua_value(x) for x in v) + ' }'


def lua_setup_cards(sc):
    """The setup as lib_eds.lua's setupCards rows (hand and field), plus the other piles as `piles`."""
    rows, piles = [], []
    for side, place, cards in parse_setup(sc['setup']):
        player_no = 0 if side == 'you' else 1
        if place == 'hand':
            rows += [{'player': player_no, 'zone': 'hand', 'slot': k, 'name': c} for k, (c, _, _) in enumerate(cards)]
        elif place[0] in 'ms' and place[-1].isdigit() or place == 'fz':
            c, pos, ss = cards[0]
            zone = 'monster' if place[0] == 'm' else 'st' if place != 'fz' else 'field'
            row = {'player': player_no, 'zone': zone, 'slot': int(place[-1]) if place != 'fz' else 0, 'name': c}
            if pos == 'set':
                row['set'] = True
            if pos == 'def':
                row['position'] = 'def'
            if ss:
                row['specialSummoned'] = True
            rows.append(row)
        else:
            piles.append({'player': player_no, 'pile': place, 'names': [c for c, _, _ in cards]})
    return rows, piles
