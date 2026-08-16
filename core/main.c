#include "tuxberry.h"

static void halt(void)
{
	for (;;)
		__asm__ volatile("wfe");
}

void tuxberry_main(uint32_t r0, uint32_t machtype, void *dtb)
{
	struct tb_simplefb fb;

	(void)r0;

	if (!fdt_valid(dtb))
		halt();

	if (!fdt_find_simplefb(dtb, &fb))
		halt();

	if (!console_init_from_fb(&fb))
		halt();

	console_puts("TUXBERRY 0.3-DEV\n");
	console_puts("------------------------------\n");
	console_puts("[BOOT] ENTRY OK\n");
	console_puts("[DTB ] OK\n");
	console_puts("[DISP] OK\n");
	console_puts("[INPUT] STARTING\n");

	/*
	 * Interactive bootloader UI.
	 */
	menu_run(dtb, &fb, machtype);

	halt();
}
