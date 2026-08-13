#include "tuxberry.h"

#define MENU_COUNT 6

static const char *menu_items[MENU_COUNT] = {
	"DEVICE INFO",
	"DTB INFO",
	"DISPLAY INFO",
	"STORAGE INFO",
	"BOOT LINUX",
	"REBOOT",
};

static void wait_return(void)
{
	console_puts("\nHOME RETURN");

	while (input_wait() != TB_KEY_SELECT)
		;
}

static void render_menu(unsigned selected)
{
	console_clear();

	console_puts("TUXBERRY 0.3-DEV\n");
	console_puts("------------------------------\n\n");

	for (unsigned i = 0; i < MENU_COUNT; i++) {
		if (i == selected)
			console_puts("> ");
		else
			console_puts("  ");

		console_puts(menu_items[i]);
		console_putc('\n');
	}

	console_puts("\nVOL UP NEXT\n");
	console_puts("HOME SELECT\n");
}

static void show_device_info(void)
{
	console_clear();

	console_puts("DEVICE INFO\n");
	console_puts("------------------------------\n\n");

	console_puts("BOARD   GT58WIFI\n");
	console_puts("MODEL   SM-T350\n");
	console_puts("SOC     MSM8916\n");
	console_puts("ARCH    ARM32\n");

	console_puts("\nINPUT\n");
	console_puts("VOL UP  GPIO107\n");
	console_puts("HOME    GPIO109\n");
	console_puts("VOL DN  PM8916 RESIN\n");

	wait_return();
}

static void show_dtb_info(void *dtb)
{
	console_clear();

	console_puts("DTB INFO\n");
	console_puts("------------------------------\n\n");

	console_puts("ADDRESS ");
	console_hex((uint32_t)(uintptr_t)dtb);
	console_putc('\n');

	console_puts("SIZE    ");
	console_hex(fdt_size(dtb));
	console_putc('\n');

	console_puts("VERSION ");
	console_hex(fdt_version(dtb));
	console_putc('\n');

	console_puts("STRUCT  ");
	console_hex(fdt_struct_offset(dtb));
	console_putc('\n');

	console_puts("STRINGS ");
	console_hex(fdt_strings_offset(dtb));
	console_putc('\n');

	wait_return();
}

static void show_display_info(const struct tb_simplefb *fb)
{
	console_clear();

	console_puts("DISPLAY INFO\n");
	console_puts("------------------------------\n\n");

	console_puts("BASE    ");
	console_hex(fb->base);
	console_putc('\n');

	console_puts("SIZE    ");
	console_hex(fb->size);
	console_putc('\n');

	console_puts("WIDTH   ");
	console_dec(fb->width);
	console_putc('\n');

	console_puts("HEIGHT  ");
	console_dec(fb->height);
	console_putc('\n');

	console_puts("STRIDE  ");
	console_dec(fb->stride);
	console_putc('\n');

	console_puts("FORMAT  ");
	console_puts(fb->format);
	console_putc('\n');

	wait_return();
}

static void show_storage_info(void)
{
	struct tb_sdhci_info s;
	struct tb_mmc_status m;

	console_clear();

	console_puts("STORAGE INFO\n");
	console_puts("------------------------------\n\n");

	if (!msm8916_sdhci_probe(&s)) {
		console_puts("SDCC1 HOST FAIL\n");
		wait_return();
		return;
	}

	console_puts("SDCC1 HOST OK\n\n");

	console_puts("HC BASE ");
	console_hex(s.hc_base);
	console_putc('\n');

	console_puts("CORE    ");
	console_hex(s.core_base);
	console_putc('\n');

	console_puts("VERSION ");
	console_hex(s.host_version);
	console_putc('\n');

	console_puts("PRESENT ");
	console_hex(s.present_state);
	console_putc('\n');

	console_puts("HOSTCTL ");
	console_hex(s.host_control);
	console_putc('\n');

	console_puts("POWER   ");
	console_hex(s.power_control);
	console_putc('\n');

	console_puts("CLOCK   ");
	console_hex(s.clock_control);
	console_putc('\n');

	console_puts("INT     ");
	console_hex(s.int_status);
	console_putc('\n');

	console_puts("CAPS    ");
	console_hex(s.capabilities);
	console_putc('\n');

	console_puts("CAPS1   ");
	console_hex(s.capabilities_1);
	console_putc('\n');

	console_puts("\nGCC CLOCKS\n");

	console_puts("SRC CMD ");
	console_hex(s.gcc_cmd_rcgr);
	console_putc('\n');

	console_puts("APPS    ");
	console_hex(s.gcc_apps_cbcr);
	console_putc('\n');

	console_puts("AHB     ");
	console_hex(s.gcc_ahb_cbcr);
	console_putc('\n');

	console_puts("\nMMC STATUS\n");
	console_puts("RESET ALL TEST\n");

	{
		int ok = msm8916_mmc_test_rca(1, &m);

		console_puts("RESULT  ");
		console_puts(ok ? "TRUE\n" : "FALSE\n");

		console_puts("RCA     ");
		console_hex(m.rca);
		console_putc('\n');

		console_puts("DIAG    ");
		console_hex(m.diag_code);
		console_putc('\n');

		console_puts("RESETB  ");
		console_hex(m.reset_before);
		console_putc('\n');

		console_puts("RESETA  ");
		console_hex(m.reset_after);
		console_putc('\n');

		console_puts("RDIAG   ");
		console_hex(m.reset_diag);
		console_putc('\n');


		console_puts("PSTATE  ");
		console_hex(m.present_state);
		console_putc('\n');

		console_puts("IRQ     ");
		console_hex(m.int_status);
		console_putc('\n');

		console_puts("RESP0   ");
		console_hex(m.response);
		console_putc('\n');

		if (m.diag_code == 0x10u)
			console_puts("REASON  RESET COMPLETE\n");
		else if (m.diag_code == 0x11u)
			console_puts("REASON  RESET TIMEOUT\n");
		else
			console_puts("REASON  UNKNOWN\n");

		console_puts(ok ? "RESET   OK\n" : "RESET   FAIL\n");
	}

	wait_return();
}


static void show_penguinberry(void)
{
	console_puts(
"   .--.         .-.        \n"
"  |o_o |       /_ _\\\\      \n"
"  |:_/ |       \\_v_/      \n"
" //   \\\\ \\\\       /         \n"
"( |     | )   strawberry   \n"
"/'\\_   _/`                  \n"
"\\___)=(___/                 \n"
"\n");
}

static void show_linux(void)
{
	console_clear();

	console_puts("BOOT LINUX\n");
	console_puts("------------------------------\n\n");

	show_penguinberry();

	console_puts("NOT IMPLEMENTED YET\n");
	console_puts("TARGET V0.4\n");

	wait_return();
}

void menu_run(void *dtb, const struct tb_simplefb *fb)
{
	unsigned selected = 0;

	for (;;) {
		enum tb_key key;

		render_menu(selected);

		key = input_wait();

		if (key == TB_KEY_NEXT) {
			selected++;

			if (selected >= MENU_COUNT)
				selected = 0;

			continue;
		}

		if (key != TB_KEY_SELECT)
			continue;

		switch (selected) {
		case 0:
			show_device_info();
			break;

		case 1:
			show_dtb_info(dtb);
			break;

		case 2:
			show_display_info(fb);
			break;

		case 3:
			show_storage_info();
			break;

		case 4:
			show_linux();
			break;

		case 5:
			console_clear();
			console_puts("REBOOTING\n");
			platform_reboot();
			break;
		}
	}
}
