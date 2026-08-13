#include "tuxberry.h"

static void halt(void)
{
	for (;;)
		__asm__ volatile("wfe");
}

void tuxberry_main(uint32_t r0, uint32_t machtype, void *dtb)
{
	struct tb_simplefb fb;

	/*
	 * No framebuffer assumptions anymore.
	 *
	 * First validate the DTB passed by lk2nd, then discover
	 * simple-framebuffer, then start the console.
	 */
	if (!fdt_valid(dtb))
		halt();

	if (!fdt_find_simplefb(dtb, &fb))
		halt();

	if (!console_init_from_fb(&fb))
		halt();

	console_puts("TUXBERRY 0.2-DEV\n");
	console_puts("------------------------------\n");

	console_puts("[BOOT] ENTRY        OK\n");

	console_puts("[BOOT] R0           ");
	console_hex(r0);
	console_putc('\n');

	console_puts("[CPU ] MACHINE      ");
	console_hex(machtype);
	console_putc('\n');

	console_puts("[DTB ] ADDRESS      ");
	console_hex((uint32_t)(uintptr_t)dtb);
	console_putc('\n');

	console_puts("[DTB ] MAGIC        OK\n");

	console_puts("[DTB ] SIZE         ");
	console_hex(fdt_size(dtb));
	console_putc('\n');

	console_puts("[DISP] SIMPLEFB     OK\n");

	console_puts("[DISP] BASE         ");
	console_hex(fb.base);
	console_putc('\n');

	console_puts("[DISP] WIDTH        ");
	console_dec(fb.width);
	console_putc('\n');

	console_puts("[DISP] HEIGHT       ");
	console_dec(fb.height);
	console_putc('\n');

	console_puts("[DISP] STRIDE       ");
	console_dec(fb.stride);
	console_putc('\n');

	console_puts("[DISP] FORMAT       ");
	console_puts(fb.format);
	console_putc('\n');

	console_puts("\n[MMC ] NOT INIT\n");
	console_puts("[USB ] NOT INIT\n");
	console_puts("[BOOT] LINUX NOT IMPLEMENTED\n");

	console_puts("\nTUXBERRY>");

	halt();
}
