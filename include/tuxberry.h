#ifndef TUXBERRY_H
#define TUXBERRY_H

#include <stdint.h>
#include <stddef.h>

#define TB_FB_BASE   0x8e000000u
#define TB_FB_WIDTH  768
#define TB_FB_HEIGHT 1024
#define TB_FB_STRIDE 2304

void console_init(void);
void console_clear(void);
void console_putc(char c);
void console_puts(const char *s);
void console_hex(uint32_t v);

void tuxberry_main(uint32_t r0, uint32_t machtype, void *dtb);

#endif
