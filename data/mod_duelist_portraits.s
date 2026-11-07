@ The original duelists' opponent-select portraits, cut out of the page pictures (src/mod_opponents.c
@ DrawSlotPortrait): 25 entries (page * 5 + slot) of 256 colours and a 56x56 8bpp tiled image, built by
@ tools/duelist_portraits.py. Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModDuelistPortraits
gModDuelistPortraits:
	.incbin "build/assets/duelist_portraits.bin"
	.balign 4
