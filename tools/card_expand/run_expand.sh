#!/bin/sh
# Run script for tools/card_expand/expand_rom.py: every input and every card-table offset in one place.
# Run it from the repository root:  sh tools/card_expand/run_expand.sh
# Edit the values below. An empty destination means "the default" (see python3 tools/card_expand/expand_rom.py -h).
set -e

ROM=EDS_Kertra.gba                                   # original or edited ROM (8-32 MiB)
OUT=build/card_expand/expanded.gba            # also writes build/card_expand/expanded_cards.csv
LIST=new_cards.txt                            # cards to add after the leftover-effect cards ("" for none)
LAST_ID=                                      # highest card ID ("" = the smallest that fits)
EXTRA=""                                      # more options, e.g. "--offline --move-scratch --force --no-leftovers"

# Where the card tables are in $ROM. These are the original addresses; change one if an edit moved that table.
SRC_NAMES=0x08268AC0        # 0x40 bytes per card
SRC_DESCS=0x087FAA20        # 0x1E0 bytes per card
SRC_ART=0x08276300          # 0x10E0 bytes per card
SRC_PALETTES=0x086067E0     # 0x80 bytes per card
SRC_STATS=0x0822CC00        # 4 bytes per card
SRC_ID2N=0x0822C4E0         # 2 bytes per card (card ID -> card number)
SRC_PASSWORDS=0x08623080    # 4 bytes per card
SRC_N2ID=0x08623DF4         # 2048 x 2 bytes (card number -> card ID)

# Where to write them in $OUT. Empty = default:
#   names, descs, id2n, n2id: in place (their source address)
#   stats: the old palettes; passwords: right after the stats
#   art: the first blank space at or after 0x08800000; palettes: right after the art
DST_NAMES=
DST_DESCS=
DST_ART=                    # empty: placed after the tables that cannot grow in place (0x08860000 overlaps the descs)
DST_PALETTES=
DST_STATS=
DST_ID2N=
DST_PASSWORDS=
DST_N2ID=

opt() { if [ -n "$2" ]; then printf ' %s %s' "$1" "$2"; fi; }
ARGS="--rom $ROM --out $OUT --list $LIST"
ARGS="$ARGS --src-names $SRC_NAMES --src-descs $SRC_DESCS --src-art $SRC_ART --src-palettes $SRC_PALETTES"
ARGS="$ARGS --src-stats $SRC_STATS --src-id2n $SRC_ID2N --src-passwords $SRC_PASSWORDS --src-n2id $SRC_N2ID"
ARGS="$ARGS$(opt --names "$DST_NAMES")$(opt --descs "$DST_DESCS")$(opt --art "$DST_ART")$(opt --palettes "$DST_PALETTES")"
ARGS="$ARGS$(opt --stats "$DST_STATS")$(opt --id2n "$DST_ID2N")$(opt --passwords "$DST_PASSWORDS")$(opt --n2id "$DST_N2ID")"
[ -z "$LIST" ] && ARGS=$(printf '%s' "$ARGS" | sed 's/--list *--src/--list "" --src/')

# shellcheck disable=SC2086
eval python3 tools/card_expand/expand_rom.py $LAST_ID $ARGS $EXTRA
