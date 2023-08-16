/* Copyright 2023 Manuel Jedinger
 * https://github.com/manujedi
 * Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

ENTRY(HandlerReset)

_estack = 0x20004000;    /* end of 16K RAM */

_Min_Heap_Size = 0;      /* required amount of heap  */
_Min_Stack_Size = 0x80;  /* required amount of stack */

MEMORY
{
	FLASH (rx)      : ORIGIN = 0x00000000, LENGTH = 60K
	RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 16K
}

SECTIONS
{
	/* Program code */
	.text :
	{
		. = ALIGN(4);
		*(.text.isr)           /* .text sections of code  */
		*(.text)           /* .text sections of code  */
		*(.text*)          /* .text* sections of code */
		*(.rodata)         /* .rodata sections        */
		*(.rodata*)        /* .rodata* sections       */
		*(.glue_7)         /* Glue arm to thumb code  */
		*(.glue_7t)        /* Glue thumb to arm code  */
		*(.eh_frame)

		KEEP(*(.fini))
		. = ALIGN(4);
		_etext = .;        /* global symbols at end   */
	} >FLASH


	/* Data sections go into RAM */
	.data :
	{
		. = ALIGN(4);
		_sdata = .;        /* Global symbol at data start */
		*(.data)           /* .data sections              */
		*(.data*)          /* .data* sections             */

		. = ALIGN(4);
		_edata = .;        /* Global symbol at data end   */
	} >RAM AT> FLASH
	 _data_flash = LOADADDR(.data);

    /* sram sections go into RAM */
    .sram_overlay :
    {
        . = ALIGN(4);
        _sram_overlay_start_ram = .;        /* Global symbol at sram_overlay start */
        *(.sram_overlay)           /* .sram_overlay sections              */
        *(.sram_overlay*)          /* .sram_overlay* sections             */

        . = ALIGN(4);
        _sram_overlay_end_ram = .;        /* Global symbol at sram_overlay end   */
    } >RAM AT> FLASH
    _sram_overlay_flash = LOADADDR(.sram_overlay);

	/* Uninitialized data */
	. = ALIGN(4);
	.bss :
	{
		_sbss = .;         /* Global symbol at bss start */
		__bss_start__ = _sbss;
		*(.bss)
		*(.bss*)
		*(COMMON)

		. = ALIGN(4);
		_ebss = .;         /* Global symbol at bss end */
		__bss_end__ = _ebss;
	} >RAM

	/* Check that there is enough RAM */
	._user_heap_stack :
	{
		. = ALIGN(4);
		. = . + _Min_Heap_Size;
		. = . + _Min_Stack_Size;
		. = ALIGN(4);
	} >RAM
}

