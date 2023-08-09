	.cpu cortex-m0
	.fpu softvfp
	.thumb

	.global sram_overlay_bin
	.text

sram_overlay_bin:
	.incbin "sram-overlay.bin"

