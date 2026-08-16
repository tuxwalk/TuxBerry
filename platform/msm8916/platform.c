/* SPDX-FileCopyrightText: 2026 Artem Novak */
/* SPDX-License-Identifier: MIT */
#include "tuxberry.h"

#define MSM8916_TLMM_BASE   0x01000000u
#define MSM8916_GPIO_STRIDE 0x00001000u
#define MSM8916_GPIO_IO     0x00000004u

#define MSM8916_PS_HOLD     0x004ab000u

static uint32_t mmio_read32(uint32_t addr)
{
	return *(volatile uint32_t *)(uintptr_t)addr;
}

static void mmio_write32(uint32_t addr, uint32_t value)
{
	*(volatile uint32_t *)(uintptr_t)addr = value;
}

int msm8916_gpio_get(unsigned gpio)
{
	uint32_t addr =
		MSM8916_TLMM_BASE +
		gpio * MSM8916_GPIO_STRIDE +
		MSM8916_GPIO_IO;

	/*
	 * MSM8916 GPIO input state is bit 0 of GPIO_IN_OUT.
	 */
	return (mmio_read32(addr) & 1u) ? 1 : 0;
}

void platform_reboot(void)
{
	/*
	 * Qualcomm PS_HOLD:
	 * dropping this to zero requests restart/power-off.
	 */
	mmio_write32(MSM8916_PS_HOLD, 0);

	__asm__ volatile("dsb sy");
	__asm__ volatile("isb");

	for (;;)
		__asm__ volatile("wfe");
}
