/* SPDX-FileCopyrightText: 2026 Artem Novak */
/* SPDX-License-Identifier: MIT */
#include "tuxberry.h"

static volatile uint8_t *fb_base;
static uint32_t fb_width;
static uint32_t fb_height;
static uint32_t fb_stride;
static uint32_t fb_bytespp;

static unsigned cursor_x;
static unsigned cursor_y;

static int streq(const char *a, const char *b)
{
	while (*a && *b) {
		if (*a != *b)
			return 0;

		a++;
		b++;
	}

	return *a == *b;
}

static uint32_t format_bytespp(const char *format)
{
	if (streq(format, "r5g6b5"))
		return 2;

	if (streq(format, "r8g8b8"))
		return 3;

	if (streq(format, "x8r8g8b8") ||
	    streq(format, "a8r8g8b8") ||
	    streq(format, "x8b8g8r8") ||
	    streq(format, "a8b8g8r8"))
		return 4;

	return 0;
}

static const uint8_t *glyph(char c)
{
	static const uint8_t blank[7] = {0,0,0,0,0,0,0};

	static const uint8_t A[7] = {14,17,17,31,17,17,17};
	static const uint8_t B[7] = {30,17,17,30,17,17,30};
	static const uint8_t C[7] = {14,17,16,16,16,17,14};
	static const uint8_t D[7] = {30,17,17,17,17,17,30};
	static const uint8_t E[7] = {31,16,16,30,16,16,31};
	static const uint8_t F[7] = {31,16,16,30,16,16,16};
	static const uint8_t G[7] = {14,17,16,23,17,17,15};
	static const uint8_t H[7] = {17,17,17,31,17,17,17};
	static const uint8_t I[7] = {14,4,4,4,4,4,14};
	static const uint8_t J[7] = {7,2,2,2,18,18,12};
	static const uint8_t K[7] = {17,18,20,24,20,18,17};
	static const uint8_t L[7] = {16,16,16,16,16,16,31};
	static const uint8_t M[7] = {17,27,21,21,17,17,17};
	static const uint8_t N[7] = {17,25,21,19,17,17,17};
	static const uint8_t O[7] = {14,17,17,17,17,17,14};
	static const uint8_t P[7] = {30,17,17,30,16,16,16};
	static const uint8_t Q[7] = {14,17,17,17,21,18,13};
	static const uint8_t R[7] = {30,17,17,30,20,18,17};
	static const uint8_t S[7] = {15,16,16,14,1,1,30};
	static const uint8_t T[7] = {31,4,4,4,4,4,4};
	static const uint8_t U[7] = {17,17,17,17,17,17,14};
	static const uint8_t V[7] = {17,17,17,17,17,10,4};
	static const uint8_t W[7] = {17,17,17,21,21,21,10};
	static const uint8_t X[7] = {17,17,10,4,10,17,17};
	static const uint8_t Y[7] = {17,17,10,4,4,4,4};
	static const uint8_t Z[7] = {31,1,2,4,8,16,31};

	static const uint8_t n0[7] = {14,17,19,21,25,17,14};
	static const uint8_t n1[7] = {4,12,4,4,4,4,14};
	static const uint8_t n2[7] = {14,17,1,2,4,8,31};
	static const uint8_t n3[7] = {30,1,1,14,1,1,30};
	static const uint8_t n4[7] = {2,6,10,18,31,2,2};
	static const uint8_t n5[7] = {31,16,16,30,1,1,30};
	static const uint8_t n6[7] = {14,16,16,30,17,17,14};
	static const uint8_t n7[7] = {31,1,2,4,8,8,8};
	static const uint8_t n8[7] = {14,17,17,14,17,17,14};
	static const uint8_t n9[7] = {14,17,17,15,1,1,14};

	static const uint8_t dash[7]  = {0,0,0,31,0,0,0};
	static const uint8_t dot[7]   = {0,0,0,0,0,12,12};
	static const uint8_t colon[7] = {0,12,12,0,12,12,0};
	static const uint8_t lb[7]    = {14,8,8,8,8,8,14};
	static const uint8_t rb[7]    = {14,2,2,2,2,2,14};
	static const uint8_t gt[7]    = {16,8,4,2,4,8,16};
	static const uint8_t slash[7] = {1,2,2,4,8,8,16};

	if (c >= 'a' && c <= 'z')
		c = (char)(c - 'a' + 'A');

	switch (c) {
	case 'A': return A; case 'B': return B;
	case 'C': return C; case 'D': return D;
	case 'E': return E; case 'F': return F;
	case 'G': return G; case 'H': return H;
	case 'I': return I; case 'J': return J;
	case 'K': return K; case 'L': return L;
	case 'M': return M; case 'N': return N;
	case 'O': return O; case 'P': return P;
	case 'Q': return Q; case 'R': return R;
	case 'S': return S; case 'T': return T;
	case 'U': return U; case 'V': return V;
	case 'W': return W; case 'X': return X;
	case 'Y': return Y; case 'Z': return Z;

	case '0': return n0; case '1': return n1;
	case '2': return n2; case '3': return n3;
	case '4': return n4; case '5': return n5;
	case '6': return n6; case '7': return n7;
	case '8': return n8; case '9': return n9;

	case '-': return dash;
	case '.': return dot;
	case ':': return colon;
	case '[': return lb;
	case ']': return rb;
	case '>': return gt;
	case '/': return slash;

	default:
		return blank;
	}
}

static void pixel(unsigned x, unsigned y, int on)
{
	volatile uint8_t *p;

	if (!fb_base)
		return;

	if (x >= fb_width || y >= fb_height)
		return;

	p = fb_base + y * fb_stride + x * fb_bytespp;

	if (fb_bytespp == 2) {
		uint16_t value = on ? 0xffffu : 0x0000u;

		p[0] = (uint8_t)value;
		p[1] = (uint8_t)(value >> 8);
		return;
	}

	p[0] = on ? 0xff : 0x00;
	p[1] = on ? 0xff : 0x00;
	p[2] = on ? 0xff : 0x00;

	if (fb_bytespp == 4)
		p[3] = on ? 0xff : 0x00;
}

static void draw_char(unsigned x, unsigned y, char c)
{
	const uint8_t *g = glyph(c);

	for (unsigned row = 0; row < 7; row++) {
		for (unsigned col = 0; col < 5; col++) {
			int on = g[row] & (1u << (4 - col));

			for (unsigned sy = 0; sy < 2; sy++) {
				for (unsigned sx = 0; sx < 2; sx++) {
					pixel(
						x + col * 2 + sx,
						y + row * 2 + sy,
						on
					);
				}
			}
		}
	}
}

int console_init_from_fb(const struct tb_simplefb *fb)
{
	uint32_t bytespp;

	if (!fb)
		return 0;

	if (!fb->base ||
	    !fb->width ||
	    !fb->height ||
	    !fb->stride)
		return 0;

	bytespp = format_bytespp(fb->format);

	if (!bytespp)
		return 0;

	if (fb->stride < fb->width * bytespp)
		return 0;

	fb_base = (volatile uint8_t *)(uintptr_t)fb->base;
	fb_width = fb->width;
	fb_height = fb->height;
	fb_stride = fb->stride;
	fb_bytespp = bytespp;

	cursor_x = 12;
	cursor_y = 12;

	console_clear();

	return 1;
}

void console_clear(void)
{
	if (!fb_base)
		return;

	for (uint32_t y = 0; y < fb_height; y++) {
		volatile uint8_t *p =
			fb_base + y * fb_stride;

		for (uint32_t x = 0; x < fb_stride; x++)
			p[x] = 0;
	}

	cursor_x = 12;
	cursor_y = 12;
}

void console_set_cursor(unsigned x, unsigned y)
{
        if (!fb_base)
                return;

        cursor_x = x;
        cursor_y = y;
}

void console_clear_line(unsigned y)
{
        unsigned end;

        if (!fb_base || y >= fb_height)
                return;

        end = y + 20;

        if (end > fb_height)
                end = fb_height;

        for (unsigned py = y; py < end; py++) {
                volatile uint8_t *p =
                        fb_base + py * fb_stride;

                for (uint32_t x = 0; x < fb_stride; x++)
                        p[x] = 0;
        }
}

void console_putc(char c)
{
	if (!fb_base)
		return;

	if (c == '\n') {
		cursor_x = 12;
		cursor_y += 20;
		return;
	}

	if (cursor_x + 12 >= fb_width) {
		cursor_x = 12;
		cursor_y += 20;
	}

	if (cursor_y + 16 >= fb_height)
		return;

	draw_char(cursor_x, cursor_y, c);
	cursor_x += 12;
}

void console_puts(const char *s)
{
	if (!s)
		return;

	while (*s)
		console_putc(*s++);
}

void console_hex(uint32_t value)
{
	static const char hex[] = "0123456789ABCDEF";

	console_puts("0X");

	for (int shift = 28; shift >= 0; shift -= 4)
		console_putc(hex[(value >> shift) & 0xf]);
}

void console_dec(uint32_t value)
{
	char buf[11];
	unsigned len = 0;

	if (!value) {
		console_putc('0');
		return;
	}

	while (value) {
		buf[len++] = '0' + (value % 10);
		value /= 10;
	}

	while (len)
		console_putc(buf[--len]);
}
