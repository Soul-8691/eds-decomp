@ Precomputed format-pack roulette tables (tools/format_packs.py, FORMAT_PACK_ROULETTE_SIZE entries per format).
@ Linked as @rodata mod_format_pack_roulettes (units.txt).

	.section .rodata
	.balign 4
	.global gFormatPackRouletteBlob
gFormatPackRouletteBlob:
	.incbin "build/assets/format_pack_roulettes.bin"
	.balign 4
