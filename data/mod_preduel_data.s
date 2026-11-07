@ The pre-duel pictures (src/mod_preduel.c): each the 224 colours of BG palette entries 32-255, the QR code (size,
@ palette, 4bpp sprite), then the 240x96 picture as 8bpp tiles, built by tools/preduel_image.py from duelists/<name>.png and
@ duelists/<name>.svg (Makefile: PREDUEL_NAMES). Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModPreDuelYugiKaiba
gModPreDuelYugiKaiba:
	.incbin "build/assets/preduel_yugi-kaiba.bin"
	.balign 4
	.global gModPreDuelCritter
gModPreDuelCritter:
	.incbin "build/assets/preduel_critter.bin"
	.balign 4
	.global gModPreDuelTreasure
gModPreDuelTreasure:
	.incbin "build/assets/preduel_treasure.bin"
	.balign 4
	.global gModPreDuelImperial
gModPreDuelImperial:
	.incbin "build/assets/preduel_imperial.bin"
	.balign 4
	.global gModPreDuelAndroid
gModPreDuelAndroid:
	.incbin "build/assets/preduel_android.bin"
	.balign 4
