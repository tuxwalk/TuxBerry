#include "tuxberry.h"

/*
 * Samsung gt58wifi:
 *
 * Volume Up = TLMM GPIO 107, active low
 * Home      = TLMM GPIO 109, active low
 *
 * Volume Down is PM8916 RESIN and intentionally not handled here yet.
 */

#define GPIO_VOLUME_UP 107u
#define GPIO_HOME      109u

static void delay(void)
{
	for (volatile unsigned i = 0; i < 100000; i++)
		__asm__ volatile("nop");
}

static enum tb_key input_raw(void)
{
	/*
	 * Active-low buttons.
	 *
	 * Give Home priority if both are held.
	 */
	if (!msm8916_gpio_get(GPIO_HOME))
		return TB_KEY_SELECT;

	if (!msm8916_gpio_get(GPIO_VOLUME_UP))
		return TB_KEY_NEXT;

	return TB_KEY_NONE;
}

enum tb_key input_wait(void)
{
	for (;;) {
		enum tb_key key = input_raw();

		if (key == TB_KEY_NONE)
			continue;

		/*
		 * Cheap polling debounce for now.
		 */
		delay();

		if (input_raw() != key)
			continue;

		/*
		 * One event per physical press.
		 */
		while (input_raw() != TB_KEY_NONE)
			;

		delay();

		return key;
	}
}
