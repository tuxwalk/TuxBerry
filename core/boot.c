/* SPDX-License-Identifier: MIT */
#include "tuxberry.h"

#define ARM_ZIMAGE_MAGIC  0x016f2818u

/*
 * TuxBerry executes from 0x80008000.
 *
 * Run the compressed Linux zImage well above TuxBerry so the
 * decompressor can safely expand the kernel into low RAM.
 */
#define LINUX_ZIMAGE_ADDR 0x81000000u
#define LINUX_DTB_ADDR    0x81f00000u
#define MAX_DTB_SIZE      0x00100000u

extern const unsigned char _binary_payload_gt58wifi_zImage_start[];
extern const unsigned char _binary_payload_gt58wifi_zImage_end[];

static uint32_t read_le32(const unsigned char *p)
{
        return ((uint32_t)p[0]) |
               ((uint32_t)p[1] << 8) |
               ((uint32_t)p[2] << 16) |
               ((uint32_t)p[3] << 24);
}

static void copy_bytes(void *dst, const void *src, uint32_t size)
{
        volatile unsigned char *d = (volatile unsigned char *)dst;
        const unsigned char *s = (const unsigned char *)src;

        while (size--)
                *d++ = *s++;
}

int tb_boot_linux(void *dtb, uint32_t machtype)
{
        const unsigned char *payload =
                _binary_payload_gt58wifi_zImage_start;

        uint32_t payload_size =
                (uint32_t)(_binary_payload_gt58wifi_zImage_end -
                           _binary_payload_gt58wifi_zImage_start);

        uint32_t magic;
        uint32_t kernel_size;
        uint32_t dtb_size;

        if (payload_size < 0x30u) {
                console_puts("KERNEL  TOO SMALL\n");
                return 0;
        }

        magic = read_le32(payload + 0x24);

        if (magic != ARM_ZIMAGE_MAGIC) {
                console_puts("KERNEL  BAD MAGIC\n");
                return 0;
        }

        /*
         * ARM zImage header:
         *
         * 0x24 magic
         * 0x28 start
         * 0x2c end
         *
         * Our known-good image uses start=0, therefore end is the
         * size of the compressed zImage before its appended DTB.
         */
        kernel_size = read_le32(payload + 0x2c);

        if (kernel_size == 0 || kernel_size > payload_size) {
                console_puts("KERNEL  BAD SIZE\n");
                return 0;
        }

        if (!fdt_valid(dtb)) {
                console_puts("DTB     INVALID\n");
                return 0;
        }

        dtb_size = fdt_size(dtb);

        if (dtb_size == 0 || dtb_size > MAX_DTB_SIZE) {
                console_puts("DTB     BAD SIZE\n");
                return 0;
        }

        console_puts("KERNEL  OK\n");

        console_puts("SIZE    ");
        console_hex(kernel_size);
        console_putc('\n');

        console_puts("LOAD    ");
        console_hex(LINUX_ZIMAGE_ADDR);
        console_putc('\n');

        console_puts("DTB     ");
        console_hex(LINUX_DTB_ADDR);
        console_putc('\n');

        console_puts("DTBSIZE ");
        console_hex(dtb_size);
        console_putc('\n');

        console_puts("\nCOPY KERNEL...\n");

        copy_bytes((void *)(uintptr_t)LINUX_ZIMAGE_ADDR,
                   payload,
                   kernel_size);

        console_puts("COPY DTB...\n");

        copy_bytes((void *)(uintptr_t)LINUX_DTB_ADDR,
                   dtb,
                   dtb_size);

        console_puts("STARTING LINUX\n");

        /*
         * Linux ARM boot protocol:
         *
         * r0 = 0
         * r1 = machine type received from lk2nd
         * r2 = physical DTB address
         *
         * Disable interrupts before handing control to Linux.
         */
        __asm__ volatile(
                "cpsid if\n"
                "dsb sy\n"
                "isb\n"
                :
                :
                : "memory"
        );

        __asm__ volatile(
                "mov r0, #0\n"
                "mov r1, %0\n"
                "mov r2, %1\n"
                "bx  %2\n"
                :
                : "r"(machtype),
                  "r"(LINUX_DTB_ADDR),
                  "r"(LINUX_ZIMAGE_ADDR)
                : "r0", "r1", "r2", "memory"
        );

        /*
         * Linux should never return.
         */
        return 0;
}
