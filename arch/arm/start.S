/* SPDX-FileCopyrightText: 2026 Artem Novak */
/* SPDX-License-Identifier: MIT */
.syntax unified
.cpu cortex-a53
.arm

.section .text.boot, "ax"

.global _image_start
.global _start
.global _image_end
.extern tuxberry_main

_image_start:
	.rept 8
	mov	r0, r0
	.endr

	b	_start

	/* ARM zImage header */
	.word	0x016f2818
	.word	0x00000000
	.word	_image_end - _image_start
	.word	0x04030201

.align 5

_start:
	/*
	 * lk2nd passes:
	 * r0 = 0
	 * r1 = machine type
	 * r2 = DTB
	 */

	/* Give C its own stack contained inside TuxBerry. */
	ldr	sp, =_stack_top

	bl	tuxberry_main

1:
	wfe
	b	1b

.ltorg

/*
 * 16 KiB stack included inside the loaded kernel image.
 */
.balign 16
_stack_bottom:
	.space 16384
_stack_top:

.balign 4
