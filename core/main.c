#include "tuxberry.h"

void tuxberry_main(uint32_t r0, uint32_t machtype, void *dtb)
{
    console_init();

    console_puts("TUXBERRY 0.1\n");
    console_puts("------------------------------\n");

    console_puts("[BOOT] ENTRY        OK\n");

    console_puts("[BOOT] R0           ");
    console_hex(r0);
    console_putc('\n');

    console_puts("[CPU ] MACHINE      ");
    console_hex(machtype);
    console_putc('\n');

    console_puts("[DTB ] ADDRESS      ");
    console_hex((uint32_t)dtb);
    console_putc('\n');

    console_puts("[DEV ] GT58WIFI     OK\n");

    console_puts("[DISP] FB BASE      ");
    console_hex(TB_FB_BASE);
    console_putc('\n');

    console_puts("[DISP] 768X1024     OK\n");
    console_puts("[DISP] RGB24        OK\n");

    console_puts("[MMC ] NOT INIT\n");
    console_puts("[USB ] NOT INIT\n");

    console_putc('\n');
    console_puts("TUXBERRY>");

    for (;;)
        __asm__ volatile("wfe");
}
