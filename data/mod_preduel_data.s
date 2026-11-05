@ The pre-duel pictures (src/mod_preduel.c): each the 224 colours of BG palette entries 32-255, then the LZSS
@ 240x96 bitmap, built by tools/preduel_image.py from the PNG in the repository root. Not part of the original ROM;
@ .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModPreDuelYugiKaiba
gModPreDuelYugiKaiba:
	.incbin "build/assets/preduel_yugi_kaiba.bin"
	.balign 4
