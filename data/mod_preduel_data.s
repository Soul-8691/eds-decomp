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

@ By duelist ID (tools/preduel_text.py, from duelists/preduel_text.txt): { picture, qr, scene } (src/mod_preduel.c
@ struct PreDuelEntry), then the text, 5 lines of 48 bytes each.
	.balign 4
	.global gModPreDuelTable
gModPreDuelTable:
	.include "build/assets/preduel_table.inc"
	.global gModPreDuelText
gModPreDuelText:
	.incbin "build/assets/preduel_text.bin"
	.balign 4

@ The original pre-duel dialogue's scenes of the opponents without a picture (src/mod_preduel.c), each 256 BG
@ colours, the 240x96 bitmap as 8bpp tiles (30 x 12), 256 OBJ colours and the 0x2000-byte eye tile sheet: zeros
@ here, filled in the linked ROM by tools/preduel_scenes.py from the scene sets (gSceneSets).
	.global gModPreDuelScenes
gModPreDuelScenes:
	.space PREDUEL_SCENE_COUNT * PREDUEL_SCENE_ENTRY
	.balign 4
