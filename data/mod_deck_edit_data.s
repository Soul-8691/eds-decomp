@ Deck Edit command-bar label for the "Empty Main/Side" action (tools/mod_deck_edit_label.py).
@ Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModDeckEditEmptyMainSideLabelGfx
gModDeckEditEmptyMainSideLabelGfx:
	.incbin "build/assets/mod_deck_edit_empty_label.bin"
	.balign 4
