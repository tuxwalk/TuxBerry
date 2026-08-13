#include "tuxberry.h"

static volatile uint8_t *const fb =
    (volatile uint8_t *)TB_FB_BASE;

static unsigned cursor_x = 12;
static unsigned cursor_y = 12;

/*
 * 5x7 glyphs.
 * Each byte represents one row; low 5 bits are pixels.
 */
static const uint8_t *glyph(char c)
{
    static const uint8_t blank[7] = {
        0,0,0,0,0,0,0
    };

    static const uint8_t A[7] = {14,17,17,31,17,17,17};
    static const uint8_t B[7] = {30,17,17,30,17,17,30};
    static const uint8_t C[7] = {14,17,16,16,16,17,14};
    static const uint8_t D[7] = {30,17,17,17,17,17,30};
    static const uint8_t E[7] = {31,16,16,30,16,16,31};
    static const uint8_t F[7] = {31,16,16,30,16,16,16};
    static const uint8_t G[7] = {14,17,16,23,17,17,15};
    static const uint8_t H[7] = {17,17,17,31,17,17,17};
    static const uint8_t I[7] = {14,4,4,4,4,4,14};
    static const uint8_t K[7] = {17,18,20,24,20,18,17};
    static const uint8_t L[7] = {16,16,16,16,16,16,31};
    static const uint8_t M[7] = {17,27,21,21,17,17,17};
    static const uint8_t N[7] = {17,25,21,19,17,17,17};
    static const uint8_t O[7] = {14,17,17,17,17,17,14};
    static const uint8_t P[7] = {30,17,17,30,16,16,16};
    static const uint8_t R[7] = {30,17,17,30,20,18,17};
    static const uint8_t S[7] = {15,16,16,14,1,1,30};
    static const uint8_t T[7] = {31,4,4,4,4,4,4};
    static const uint8_t U[7] = {17,17,17,17,17,17,14};
    static const uint8_t V[7] = {17,17,17,17,17,10,4};
    static const uint8_t Y[7] = {17,17,10,4,4,4,4};

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

    switch (c) {
    case 'A': return A; case 'B': return B;
    case 'C': return C; case 'D': return D;
    case 'E': return E; case 'F': return F;
    case 'G': return G; case 'H': return H;
    case 'I': return I; case 'K': return K;
    case 'L': return L; case 'M': return M;
    case 'N': return N; case 'O': return O;
    case 'P': return P; case 'R': return R;
    case 'S': return S; case 'T': return T;
    case 'U': return U; case 'V': return V;
    case 'Y': return Y;

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
    default:  return blank;
    }
}

static void pixel(unsigned x, unsigned y, int on)
{
    if (x >= TB_FB_WIDTH || y >= TB_FB_HEIGHT)
        return;

    volatile uint8_t *p =
        fb + y * TB_FB_STRIDE + x * 3;

    if (on) {
        /*
         * White works regardless of RGB/BGR byte ordering.
         */
        p[0] = 0xff;
        p[1] = 0xff;
        p[2] = 0xff;
    } else {
        p[0] = 0x00;
        p[1] = 0x00;
        p[2] = 0x00;
    }
}

static void draw_char(unsigned x, unsigned y, char c)
{
    const uint8_t *g = glyph(c);

    /* 2x scaling -> readable 10x14 glyph */
    for (unsigned row = 0; row < 7; row++) {
        for (unsigned col = 0; col < 5; col++) {
            int on = g[row] & (1u << (4 - col));

            for (unsigned sy = 0; sy < 2; sy++)
                for (unsigned sx = 0; sx < 2; sx++)
                    pixel(x + col * 2 + sx,
                          y + row * 2 + sy,
                          on);
        }
    }
}

void console_clear(void)
{
    for (unsigned y = 0; y < TB_FB_HEIGHT; y++) {
        volatile uint8_t *p = fb + y * TB_FB_STRIDE;

        for (unsigned x = 0; x < TB_FB_STRIDE; x++)
            p[x] = 0;
    }

    cursor_x = 12;
    cursor_y = 12;
}

void console_init(void)
{
    console_clear();
}

void console_putc(char c)
{
    if (c == '\n') {
        cursor_x = 12;
        cursor_y += 20;
        return;
    }

    if (cursor_x + 12 >= TB_FB_WIDTH) {
        cursor_x = 12;
        cursor_y += 20;
    }

    draw_char(cursor_x, cursor_y, c);
    cursor_x += 12;
}

void console_puts(const char *s)
{
    while (*s)
        console_putc(*s++);
}

void console_hex(uint32_t v)
{
    static const char h[] = "0123456789ABCDEF";

    console_puts("0X");

    for (int n = 7; n >= 0; n--)
        console_putc(h[(v >> (n * 4)) & 0xf]);
}
