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
	 * Real bytes at the end of the image.
	 *
	 * This prevents objcopy from ending the raw binary before
	 * the address advertised by the zImage header.
	 */
	.image_tail : ALIGN(4)
	{
		LONG(0x31584254) /* "TBX1" */
	}

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
