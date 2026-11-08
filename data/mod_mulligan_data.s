@ The card command menu's Mulligan icon (0x80 bytes) and label (0x100 bytes) (tools/mulligan_menu_gfx.py),
@ copied over the menu sheets' slot 13 by ModLoadMulliganMenuGfx (src/mod_mulligan.c).
@ Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModMulliganMenuGfx
gModMulliganMenuGfx:
	.incbin "build/assets/mulligan_menu_gfx.bin"
	.balign 4
