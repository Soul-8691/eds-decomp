@ Format-pack shop covers: 56x112 8bpp tiles per pack (tools/format_pack_covers.py),
@ indexed by FormatPackIndexFromId (src/mod_format_packs.c ModFormatPackLoadCoverGfx).

	.section .rodata
	.balign 4
	.global gFormatPackCovers
gFormatPackCovers:
	.incbin "build/assets/format_pack_covers.bin"
	.balign 4
