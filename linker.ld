ENTRY(_image_start)

SECTIONS
{
	. = 0x80008000;

	_image_load_start = .;

	.text : ALIGN(4)
	{
		KEEP(*(.text.boot))
		*(.text*)
	}

	.rodata : ALIGN(4)
	{
		*(.rodata*)
	}

	.data : ALIGN(4)
	{
		*(.data*)
	}

	/*
	 * End of bytes actually present in tuxberry.bin.
	 * The zImage header uses this symbol.
	 */
	. = ALIGN(4);
	_image_end = .;

	.bss (NOLOAD) : ALIGN(4)
	{
		__bss_start = .;
		*(.bss*)
		*(COMMON)
		__bss_end = .;
	}

	/DISCARD/ :
	{
		*(.comment)
		*(.note*)
		*(.ARM.attributes)
		*(.ARM.exidx*)
		*(.ARM.extab*)
	}
}
