"""Infer CPU/AI support for added cards from src/added_effects.c (and engine keys)."""
from __future__ import annotations

import re

from gencards import ident

AE_PATH = None  # set by caller


def _macro(name: str) -> str:
    return ident(name)


def _has_tag(name: str) -> str:
    return 'HAS_' + ident(name)


def _card_section(text: str, name: str, tag: str) -> str:
    """Slice from the card's `/* ---- Name` header through the next card header (not whole #ifdef span)."""
    m = re.search(rf'/\* ---- {re.escape(name)}', text)
    if not m:
        return ''
    start = m.start()
    nxt = text.find('\n/* ----', start + 5)
    end = text.find(f'#endif /* {tag} */', start)
    candidates = [x for x in (nxt, end, start + 12_000) if x > start]
    end = min(candidates) if candidates else start + 12_000
    return text[start:end]


def _cpu_comment_lines(block: str) -> list[str]:
    lines: list[str] = []
    for line in block.splitlines():
        s = line.strip()
        if not (s.startswith('* CPU') or s.startswith('CPU')):
            continue
        s = re.sub(r'^\*\s*', '', s).rstrip('*/').strip()
        if s and '*/' not in s:
            lines.append(s)
    return lines


def _function_chunk(text: str, needle: str, size: int = 120_000) -> str:
    i = text.find(needle)
    return text[i : i + size] if i >= 0 else ''


def _case_body(chunk: str, macro: str) -> str | None:
    pat = rf'case {macro}:.*?(?=\n#ifdef HAS_|\ncase CARD_|\n\s*\}}\s*\n\s*return 1;)'
    m = re.search(pat, chunk, re.DOTALL)
    return m.group(0) if m else None


def _case_notes(body: str) -> str:
    m = re.search(r'/\*\s*(.+?)\s*\*/', body)
    if m:
        return m.group(1).strip()
    first = body.split('\n', 1)[0]
    if '/*' in first:
        return first.split('/*', 1)[1].split('*/', 1)[0].strip()
    return ''


def _summarize_case(body: str, *, default_yes: bool) -> tuple[str, str]:
    """Return (tier hint, short detail)."""
    note = _case_notes(body)
    if re.search(r'\breturn\s+0\s*;', body):
        if 'only chained' in body.lower() or 'only chained' in note.lower():
            return 'dedicated', note or 'Chain-only (AiChain*); not flipped from Set blindly'
        if 'AiUse' in note or 'decides' in note.lower():
            return 'dedicated', note or 'Separate AiUse* path (not card-menu loop)'
        return 'partial', note or 'AiWants* returns 0 for one path; see src/added_effects.c'
    if 'return 1' in body or 'AiWants' in body or 'return Ai' in body:
        return 'dedicated', note or 'Conditional AiWants* / AiShouldActivateAddedCard'
    return 'general', note or ('Default yes' if default_yes else 'Listed case')


def _switch_status(text: str, func: str, macro: str, *, default_yes: bool) -> tuple[str | None, str]:
    chunk = _function_chunk(text, func)
    if not chunk:
        return None, ''
    body = _case_body(chunk, macro)
    if body:
        return _summarize_case(body, default_yes=default_yes)
    if f'case {macro}:' in chunk:
        return 'dedicated', 'Custom case'
    return None, ''


def _dedicated_symbols(block: str) -> list[str]:
    if not block:
        return []
    names = set(re.findall(r'\b(Ai(?:Use|Wants)\w+)\b', block))
    names |= set(re.findall(r'\b(\w*Cpu\w+)\b', block))
    return sorted(names)


def _in_effect_table(text: str, macro: str) -> bool:
    tail = text.split('const struct CardEffect gAddedCardEffects[]', 1)
    if len(tail) < 2:
        return False
    tag = 'HAS_' + macro
    return tag in tail[1] and macro in tail[1]


def _has_monster_activation(coded: str) -> bool:
    cl = coded.lower().replace(' ', '')
    return 'activatedeffect' in cl or 'canactivatemonstereffect' in cl or (
        'card menu' in cl and 'activate' in cl
    )


def ai_status(name: str, ae_text: str, engine: dict[str, int], coded: str, impl: str) -> tuple[str, str]:
    """Return (status line for TESTING.md, detail paragraph)."""
    macro = _macro(name)
    tag = _has_tag(name)
    block = _card_section(ae_text, name, tag)
    cpu_lines = _cpu_comment_lines(block)
    symbols = _dedicated_symbols(block)
    if not symbols:
        parts = [p for p in ident(name).lower().split('_') if len(p) > 3]
        for m in re.finditer(r'\b(Ai(?:Use|Wants|Chain)\w+)\b', ae_text):
            fn = m.group(1).lower()
            if parts and all(p in fn for p in parts):
                symbols.append(m.group(1))
        symbols = sorted(set(symbols))

    ctype_hint = ''
    if 'Spell' in impl or 'normal spell' in coded.lower():
        ctype_hint = 'spell'
    elif 'Trap' in impl or 'trap' in coded.lower():
        ctype_hint = 'trap'
    elif 'Monster' in impl or 'gAddedCardEffects' in impl:
        ctype_hint = 'monster'

    tiers: list[str] = []
    details: list[str] = []

    if cpu_lines:
        details.append('Block comment: ' + ' '.join(cpu_lines[:2]))
    if symbols:
        details.append('Hooks: ' + ', '.join(f'`{s}`' for s in symbols[:6]))
    if re.search(r'\b\w+Cpu\w+\b', block):
        tiers.append('dedicated')
        details.append('Dedicated `*Cpu*` handlers in the card\'s effect steps.')

    wm, wn = _switch_status(ae_text, 'static int AiWantsMonsterEffect', macro, default_yes=True)
    ws, sn = _switch_status(ae_text, 'static int AiWantsSpell', macro, default_yes=True)
    wa, an = _switch_status(ae_text, 'int AiShouldActivateAddedCard', macro, default_yes=True)

    for label, tier, note in (
        ('Monster menu', wm, wn),
        ('Main Phase Spell', ws, sn),
        ('Set/chain', wa, an),
    ):
        if tier:
            tiers.append(tier)
            if note:
                details.append(f'{label}: {note}')

    in_table = _in_effect_table(ae_text, macro)
    engine_only = name in engine and not block and 'Vanilla engine' in impl

    if engine_only or (not block and not in_table and 'engine effect key' in coded.lower()):
        if not _has_monster_activation(coded) and any(
            x in coded.lower()
            for x in ('battle phase', 'after it attacks', 'damage step', 'passive', 'continuous')
        ):
            return (
                '**Automatic** (no activation logic needed)',
                'Effect runs in engine/battle code when the CPU summons and attacks normally (`ai_deck.c`, '
                '`ai_summon.c`, `ai_picks.c`). Confirm the opponent still sets/attacks with the card; the passive or '
                'mandatory follow-up should apply without a card-menu prompt.',
            )
        return (
            '**Generic engine**',
            'Uses the vanilla effect key tables; the CPU plays it like other cards in its deck lists. No dedicated '
            '`AiWants*` row in `src/added_effects.c`—watch whether the opponent activates the effect when rules allow.',
        )

    if 'dedicated' in tiers or symbols or cpu_lines:
        status = '**Yes — dedicated logic**'
    elif 'partial' in tiers:
        status = '**Partial**'
    elif 'general' in tiers or (in_table and _has_monster_activation(coded)):
        status = '**Yes — generalized routine**'
    elif in_table and ctype_hint in ('spell', 'trap'):
        status = '**Yes — generalized routine**'
        details.append('`AiPlayAddedCards` / `AiTryPlaySpellTrap` + default added-card policy.')
    elif ctype_hint == 'monster' and not _has_monster_activation(coded):
        status = '**Automatic** (no card-menu activation)'
        details.append('No `CanActivateMonsterEffect` path; CPU uses standard lines.')
    elif ctype_hint in ('spell', 'trap') and in_table:
        status = '**Yes — generalized routine**'
        details.append('`AiPlayAddedCards` / `AiTryPlaySpellTrap` with default `AiWantsSpell` or '
                       '`AiShouldActivateAddedCard` (return 1).')
    else:
        status = '**Unclear / likely missing**'
        details.append('No `CPU:` notes and no switch case found; treat as untested for CPU until you confirm in a duel.')

    body = ' '.join(details) if details else 'See `src/added_effects.c` and `ai_deck.c` (`AiPlayAddedCards`).'
    return status, body


def ai_scenarios(name: str, status: str, fmt: str, *, detail: str = '') -> list[str]:
    """Short opponent-focused AI checks."""
    scenarios: list[str] = []
    base = (
        f'Campaign **{fmt}** duel (or Test Duel with the format opponent deck): give the **opponent** '
        f'`{name}` in the main/extra deck at a realistic copy count from `usage/` decks.'
    )
    if 'Partial' in status or 'missing' in status.lower() or 'Unclear' in status:
        scenarios.append(
            base + f' Watch several turns; note any turn where a human could legally use **{name}** but the CPU does '
            f'not. File gaps against `src/added_effects.c` / `ai_deck.c`.'
        )
    elif 'Automatic' in status:
        scenarios.append(
            base + f' CPU should Normal Summon/set and attack with **{name}** using ordinary lines; verify mandatory '
            f'/ passive text from **Coded behavior** without expecting a card-menu activation.'
        )
    elif 'Generic engine' in status:
        scenarios.append(
            base + f' Verify the CPU plays **{name}** when its generic spell/monster heuristics allow (same class as '
            f'vanilla cards with that effect key).'
        )
    else:
        scenarios.append(
            base + f' Let the CPU play the duel: it should activate/set/chain **{name}** when its `AiWants*` / '
            f'`AiShouldActivateAddedCard` policy says so (see **AI coverage**). Repeat until you see at least one '
            f'successful CPU use or confirm the policy never fires on this board.'
        )
        if 'Set/chain' in detail or 'AiShouldActivate' in detail:
            scenarios.append(
                f'Second pass: open a chain window the policy cares about (Normal/Special Summon, attack declaration, '
                f'or your Spell activation) and confirm the opponent flips/sets/chains **{name}** when `CanActivateFieldCard` '
                f'allows.'
            )
        else:
            scenarios.append(
                f'Second pass: simplify the board until the hook conditions in **AI coverage** are met (extra hand size, '
                f'weaker monsters, or an open monster zone) and re-check CPU behavior.'
            )
    return scenarios[:2]
