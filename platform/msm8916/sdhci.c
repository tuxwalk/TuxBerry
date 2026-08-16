/* SPDX-License-Identifier: MIT */
#include "tuxberry.h"

/*
 * MSM8916 SDCC1
 *
 * HC   = standard SDHCI register window
 * CORE = Qualcomm-specific register window
 *
 * This first v0.3 driver is deliberately READ ONLY.
 */

#define SDCC1_HC_BASE      0x07824900u
#define SDCC1_CORE_BASE    0x07824000u

/*
 * MSM8916 GCC
 */
#define MSM8916_GCC_BASE           0x01800000u
#define GCC_SDCC1_CMD_RCGR         0x00042004u
#define GCC_SDCC1_APPS_CBCR        0x00042018u
#define GCC_SDCC1_AHB_CBCR         0x0004201cu

#define SDHCI_PRESENT_STATE    0x24u
#define SDHCI_HOST_CONTROL     0x28u
#define SDHCI_POWER_CONTROL    0x29u
#define SDHCI_CLOCK_CONTROL    0x2cu
#define SDHCI_INT_STATUS       0x30u
#define SDHCI_CAPABILITIES     0x40u
#define SDHCI_CAPABILITIES_1   0x44u
#define SDHCI_HOST_VERSION     0xfeu

static uint8_t mmio_read8(uint32_t addr)
{
	return *(volatile uint8_t *)(uintptr_t)addr;
}

static uint16_t mmio_read16(uint32_t addr)
{
	return *(volatile uint16_t *)(uintptr_t)addr;
}

static uint32_t mmio_read32(uint32_t addr)
{
	return *(volatile uint32_t *)(uintptr_t)addr;
}

int msm8916_sdhci_probe(struct tb_sdhci_info *info)
{
	uint16_t version;

	if (!info)
		return 0;

	info->hc_base = SDCC1_HC_BASE;
	info->core_base = SDCC1_CORE_BASE;

	info->gcc_cmd_rcgr =
		mmio_read32(MSM8916_GCC_BASE + GCC_SDCC1_CMD_RCGR);

	info->gcc_apps_cbcr =
		mmio_read32(MSM8916_GCC_BASE + GCC_SDCC1_APPS_CBCR);

	info->gcc_ahb_cbcr =
		mmio_read32(MSM8916_GCC_BASE + GCC_SDCC1_AHB_CBCR);

	info->present_state =
		mmio_read32(SDCC1_HC_BASE + SDHCI_PRESENT_STATE);

	info->host_control =
		mmio_read8(SDCC1_HC_BASE + SDHCI_HOST_CONTROL);

	info->power_control =
		mmio_read8(SDCC1_HC_BASE + SDHCI_POWER_CONTROL);

	info->clock_control =
		mmio_read16(SDCC1_HC_BASE + SDHCI_CLOCK_CONTROL);

	info->int_status =
		mmio_read32(SDCC1_HC_BASE + SDHCI_INT_STATUS);

	info->capabilities =
		mmio_read32(SDCC1_HC_BASE + SDHCI_CAPABILITIES);

	info->capabilities_1 =
		mmio_read32(SDCC1_HC_BASE + SDHCI_CAPABILITIES_1);

	version =
		mmio_read16(SDCC1_HC_BASE + SDHCI_HOST_VERSION);

	info->host_version = version;

	/*
	 * 0xffff is typical unmapped/dead-bus garbage.
	 * Zero is also not expected on this controller.
	 */
	info->valid =
		(version != 0xffffu && version != 0x0000u);

	return info->valid;
}


#define SDHCI_ARGUMENT          0x08u
#define SDHCI_COMMAND           0x0eu
#define SDHCI_RESPONSE_0        0x10u
#define SDHCI_INT_STATUS_REG    0x30u

#define SDHCI_CMD_INHIBIT_BIT   0x00000001u

#define SDHCI_INT_RESPONSE_BIT  0x00000001u
#define SDHCI_INT_ERROR_BIT     0x00008000u
#define SDHCI_INT_ERROR_MASK    0xffff8000u

#define MMC_CMD13_FLAGS         0x001au

static uint8_t mmio_read8_sdhci(uint32_t addr)
{
	return *(volatile uint8_t *)(uintptr_t)addr;
}

static void mmio_write8_sdhci(uint32_t addr, uint8_t value)
{
	*(volatile uint8_t *)(uintptr_t)addr = value;
}

static void mmio_write16_sdhci(uint32_t addr, uint16_t value)
{
	*(volatile uint16_t *)(uintptr_t)addr = value;
}

static void mmio_write32_sdhci(uint32_t addr, uint32_t value)
{
	*(volatile uint32_t *)(uintptr_t)addr = value;
}

int msm8916_mmc_test_rca(uint16_t rca,
			struct tb_mmc_status *status)
{
	uint32_t timeout;
	uint32_t irq = 0;
	uint32_t hc_mode;
	uint32_t state;

	if (!status)
		return 0;

	status->valid = 0;
	status->rca = rca;
	status->response = 0;
	status->int_status = 0;
	status->present_state = 0;
	status->diag_code = 0;
	status->reset_before = 0;
	status->reset_after = 0;
	status->reset_diag = 0;

	/*
	 * TUXBERRY MSM8916 HC WRAPPER INIT
	 *
	 * MSM8916 SDCC1 has a Qualcomm wrapper around the standard
	 * SDHCI register block. Match the initialization performed
	 * by the upstream sdhci-msm driver.
	 *
	 * HC/core:
	 *   0x07824900 = SDHCI HC
	 *   0x07824000 = Qualcomm core
	 */

	/* CORE_VENDOR_SPEC POR value, HC + 0x10c */
	mmio_write32_sdhci(
		SDCC1_HC_BASE + 0x10cu,
		0x00000a9cu);

	/*
	 * CORE_HC_MODE at core + 0x78.
	 *
	 * First select HC mode, then disable the FF clock software
	 * reset behavior (bit 13), matching Linux sdhci-msm.
	 */
	mmio_write32_sdhci(
		0x07824000u + 0x78u,
		0x00000001u);

	hc_mode = mmio_read32(
		0x07824000u + 0x78u);

	hc_mode |= (1u << 13);

	mmio_write32_sdhci(
		0x07824000u + 0x78u,
		hc_mode);

	/*
	 * TUXBERRY SDHCI RESET DIAGNOSTIC
	 *
	 * Software Reset register = HC + 0x2f
	 * bit 0 = RESET ALL.
	 *
	 * This build intentionally returns immediately after the reset
	 * test. Do not issue CMD0/CMD13 after RESET ALL until the host
	 * has been reinitialized.
	 */
	status->reset_before =
		(uint32_t)mmio_read8_sdhci(
			SDCC1_HC_BASE + 0x2fu);

	mmio_write8_sdhci(
		SDCC1_HC_BASE + 0x2fu,
		0x01u);

	timeout = 1000000u;

	while (timeout--) {
		status->reset_after =
			(uint32_t)mmio_read8_sdhci(
				SDCC1_HC_BASE + 0x2fu);

		if (!(status->reset_after & 0x01u))
			break;
	}

	status->reset_after =
		(uint32_t)mmio_read8_sdhci(
			SDCC1_HC_BASE + 0x2fu);

	status->present_state =
		mmio_read32(
			SDCC1_HC_BASE + SDHCI_PRESENT_STATE);

	status->int_status =
		mmio_read32(
			SDCC1_HC_BASE + SDHCI_INT_STATUS_REG);

	if (status->reset_after & 0x01u) {
		status->reset_diag = 2u;
		status->diag_code = 0x11u;
		status->valid = 0;
		return 0;
	}

	status->reset_diag = 1u;
	status->diag_code = 0x10u;
	status->valid = 1;
	return 1;

	/*
	 * First make sure CMD line isn't inhibited.
	 */
	timeout = 20000;

	while (timeout--) {
		state = mmio_read32(
			SDCC1_HC_BASE + SDHCI_PRESENT_STATE);

		if (!(state & SDHCI_CMD_INHIBIT_BIT))
			break;
	}

	if (state & SDHCI_CMD_INHIBIT_BIT) {
		status->present_state = state;
		status->int_status =
			mmio_read32(SDCC1_HC_BASE + SDHCI_INT_STATUS_REG);
		status->response =
			mmio_read32(SDCC1_HC_BASE + SDHCI_RESPONSE_0);
		status->diag_code = 1;
		return 0;
	}

	/*
	 * Clear old latched interrupt bits.
	 */
	/*
	 * Enable command status generation.
	 *
	 * Bit 0     = command complete
	 * Bits16-19 = command timeout / CRC / end-bit / index errors
	 *
	 * SIGNAL_ENABLE remains zero because TuxBerry polls INT_STATUS.
	 */
	mmio_write32_sdhci(
		SDCC1_HC_BASE + 0x34u,
		0x000f0001u);

	mmio_write32_sdhci(
		SDCC1_HC_BASE + 0x38u,
		0x00000000u);

	mmio_write32_sdhci(
		SDCC1_HC_BASE + SDHCI_INT_STATUS_REG,
		0xffffffffu);

	/*
	 * Diagnostic CMD0 GO_IDLE_STATE.
	 *
	 * CMD0 has no response. If we still never get COMMAND COMPLETE,
	 * the problem is below the eMMC/RCA protocol level.
	 */
	mmio_write32_sdhci(
		SDCC1_HC_BASE + SDHCI_ARGUMENT,
		0x00000000u);

	mmio_write16_sdhci(
		SDCC1_HC_BASE + SDHCI_COMMAND,
		(uint16_t)(0u << 8));

	/*
	 * Short bounded poll.
	 */
	timeout = 50000;

	while (timeout--) {
		irq = mmio_read32(
			SDCC1_HC_BASE + SDHCI_INT_STATUS_REG);

		if (irq & SDHCI_INT_RESPONSE_BIT)
			break;

		if (irq & SDHCI_INT_ERROR_BIT)
			break;
	}

	/*
	 * Preserve the actual hardware state.  Do not replace IRQ with
	 * a software sentinel: that destroys the evidence we need.
	 */
	status->int_status = irq;
	status->present_state =
		mmio_read32(SDCC1_HC_BASE + SDHCI_PRESENT_STATE);
	status->response =
		mmio_read32(SDCC1_HC_BASE + SDHCI_RESPONSE_0);

	if (!(irq & (SDHCI_INT_RESPONSE_BIT |
		     SDHCI_INT_ERROR_BIT))) {
		status->diag_code = 2;
		return 0;
	}

	if (irq & SDHCI_INT_ERROR_MASK) {
		status->diag_code = 3;

		mmio_write32_sdhci(
			SDCC1_HC_BASE + SDHCI_INT_STATUS_REG,
			irq);

		return 0;
	}

	status->response =
		mmio_read32(SDCC1_HC_BASE + SDHCI_RESPONSE_0);

	status->valid = 1;

	mmio_write32_sdhci(
		SDCC1_HC_BASE + SDHCI_INT_STATUS_REG,
		irq);

	return 1;
}
