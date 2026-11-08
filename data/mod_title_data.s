@ The title screen's images in plain layouts (src/mod_title.c), built by tools/title_images.py: u32 offsets[4], then
@ "Yak's YGO Hack" (title/yaks_ygo_hack.png), the flames, the coin and the copyright line, each its colours then its
@ tiles in row-major order. Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModTitleImages
gModTitleImages:
	.incbin "build/assets/title_images.bin"
	.balign 4
