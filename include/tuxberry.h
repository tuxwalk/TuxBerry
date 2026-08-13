#ifndef TUXBERRY_H
#define TUXBERRY_H

#include <stdint.h>
#include <stddef.h>

struct tb_simplefb {
	uint32_t base;
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint32_t stride;
	char format[16];
};

/* Console */
int console_init_from_fb(const struct tb_simplefb *fb);
void console_clear(void);
void console_putc(char c);
void console_puts(const char *s);
void console_hex(uint32_t value);
void console_dec(uint32_t value);

/* FDT */
int fdt_valid(const void *dtb);
uint32_t fdt_size(const void *dtb);
uint32_t fdt_version(const void *dtb);
uint32_t fdt_struct_offset(const void *dtb);
uint32_t fdt_strings_offset(const void *dtb);
int fdt_find_simplefb(const void *dtb, struct tb_simplefb *fb);

/* Main */
void tuxberry_main(uint32_t r0, uint32_t machtype, void *dtb);

#endif
