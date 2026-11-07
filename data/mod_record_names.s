@ The Records screen's name plates for this build's pages (src/mod_opponents.c ModRecordDrawPage): u32 offsets[4],
@ then an image pack per page, built by tools/record_names.py from the original plates (assets/gfx/bank_b) and the
@ opponents' names in src/mod_opponents.c. Not part of the original ROM; .rodata.added (units.txt).

	.section .rodata
	.balign 4
	.global gModRecordNames
gModRecordNames:
	.incbin "build/assets/record_names.bin"
	.balign 4
