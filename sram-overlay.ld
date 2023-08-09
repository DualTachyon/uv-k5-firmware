ENTRY(overlay_FLASH_RebootToBootloader)

MEMORY
{
	SRAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 1K
}

SECTIONS
{
	. = ALIGN(4);
	.text :
	{
		overlay_text_start = .;
		*(.text)
		*(.text*)
		*(.rodata)
		*(.rodata*)
		. = ALIGN(4);
		overlay_text_end = .;
	} >SRAM

	. = ALIGN(4);
	.data :
	{
		overlay_data_start = .;
		*(.data)
		*(.data*)

		. = ALIGN(4);
		overlay_data_end = .;
	} >SRAM

	. = ALIGN(4);
	.bss :
	{
		overlay_bss_start = .;
		*(.bss)
		*(.bss*)
		*(COMMON)

		. = ALIGN(4);
		overlay_bss_end = .;
	} >SRAM
}

