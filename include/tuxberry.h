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

enum tb_key {
	TB_KEY_NONE = 0,
	TB_KEY_NEXT,
	TB_KEY_SELECT,
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


struct tb_sdhci_info {
	uint32_t hc_base;
	uint32_t core_base;

	uint32_t present_state;
	uint32_t capabilities;
	uint32_t capabilities_1;
	uint32_t int_status;

	uint16_t host_version;
	uint16_t clock_control;

	uint32_t gcc_cmd_rcgr;
	uint32_t gcc_apps_cbcr;
	uint32_t gcc_ahb_cbcr;

	uint8_t host_control;
	uint8_t power_control;

	int valid;
};


struct tb_mmc_status {
	uint16_t rca;
	uint32_t response;
	uint32_t int_status;
	uint32_t present_state;
	uint32_t diag_code;
	uint32_t reset_before;
	uint32_t reset_after;
	uint32_t reset_diag;
	int valid;
};

int msm8916_mmc_test_rca(uint16_t rca, struct tb_mmc_status *status);

int msm8916_sdhci_probe(struct tb_sdhci_info *info);

/* MSM8916 platform */
int msm8916_gpio_get(unsigned gpio);
void platform_reboot(void);

/* Device input */
enum tb_key input_wait(void);

/* Linux boot */
int tb_boot_linux(void *dtb, uint32_t machtype);

/* UI */
void menu_run(void *dtb, const struct tb_simplefb *fb, uint32_t machtype);

/* Entry */
void tuxberry_main(uint32_t r0, uint32_t machtype, void *dtb);

#endif
