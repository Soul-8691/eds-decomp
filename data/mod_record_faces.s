@ The Records screen's faces of the opponents with a card art portrait (src/mod_opponents.c DrawRecordFace): 25
@ entries by duelist ID, each u16 colours[16] and 48 x 16 pixels as 6 x 2 4bpp tiles (416 bytes). Zeros here:
@ tools/record_faces.py writes them into the ROM after linking, from each opponent's portrait card (sOpponents) and
@ the card art in that ROM. Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModRecordFaces
gModRecordFaces:
	.space 25 * 416
	.balign 4
