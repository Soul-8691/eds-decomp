@ The field card icons with art (src/mod_field_icons.c): the shared 64-colour palette, the 8bpp icon frames and
@ every card's 17x17 miniature, built by tools/cardicons.py --data from card_icons/*.png and the card art (layout
@ in that tool's docstring). Not part of the original ROM; placed with the other mod data (@rodata, units.txt).

	.section .rodata
	.balign 4
	.global gModFieldIconData
gModFieldIconData:
	.incbin "build/assets/field_icons.bin"
	.balign 4
