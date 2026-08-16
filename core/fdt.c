/* SPDX-FileCopyrightText: 2026 Artem Novak */
/* SPDX-License-Identifier: MIT */
#include "tuxberry.h"

#define FDT_MAGIC       0xd00dfeedu

#define FDT_BEGIN_NODE  1u
#define FDT_END_NODE    2u
#define FDT_PROP        3u
#define FDT_NOP         4u
#define FDT_END         9u

struct fdt_header {
	uint32_t magic;
	uint32_t totalsize;
	uint32_t off_dt_struct;
	uint32_t off_dt_strings;
	uint32_t off_mem_rsvmap;
	uint32_t version;
	uint32_t last_comp_version;
	uint32_t boot_cpuid_phys;
	uint32_t size_dt_strings;
	uint32_t size_dt_struct;
};

struct fb_node_state {
	struct tb_simplefb fb;
	int simplefb;
};

static uint32_t be32(const void *ptr)
{
	const uint8_t *p = ptr;

	return ((uint32_t)p[0] << 24) |
	       ((uint32_t)p[1] << 16) |
	       ((uint32_t)p[2] << 8) |
	       ((uint32_t)p[3]);
}

static uint32_t align4(uint32_t n)
{
	return (n + 3u) & ~3u;
}

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

static void clear_fb(struct tb_simplefb *fb)
{
	unsigned i;

	fb->base = 0;
	fb->size = 0;
	fb->width = 0;
	fb->height = 0;
	fb->stride = 0;

	for (i = 0; i < sizeof(fb->format); i++)
		fb->format[i] = 0;
}

static int compatible_has(const uint8_t *data,
			  uint32_t len,
			  const char *wanted)
{
	uint32_t pos = 0;

	while (pos < len) {
		const char *s = (const char *)(data + pos);
		uint32_t n = 0;

		while ((pos + n) < len && s[n])
			n++;

		if ((pos + n) >= len)
			return 0;

		if (streq(s, wanted))
			return 1;

		pos += n + 1;
	}

	return 0;
}

static void copy_string(char *dst,
			uint32_t dst_size,
			const uint8_t *src,
			uint32_t src_size)
{
	uint32_t i = 0;

	if (!dst_size)
		return;

	while (i + 1 < dst_size &&
	       i < src_size &&
	       src[i]) {
		dst[i] = (char)src[i];
		i++;
	}

	dst[i] = 0;
}

static void parse_reg(struct tb_simplefb *fb,
		      const uint8_t *data,
		      uint32_t len)
{
	if (len == 8) {
		/* 1 address cell + 1 size cell */
		fb->base = be32(data);
		fb->size = be32(data + 4);
		return;
	}

	if (len == 12) {
		/* 2 address cells + 1 size cell */
		uint32_t hi = be32(data);
		uint32_t lo = be32(data + 4);

		if (!hi) {
			fb->base = lo;
			fb->size = be32(data + 8);
		}
		return;
	}

	if (len >= 16) {
		/* 2 address cells + 2 size cells */
		uint32_t ahi = be32(data);
		uint32_t alo = be32(data + 4);
		uint32_t shi = be32(data + 8);
		uint32_t slo = be32(data + 12);

		if (!ahi && !shi) {
			fb->base = alo;
			fb->size = slo;
		}
	}
}

int fdt_valid(const void *dtb)
{
	const struct fdt_header *h = dtb;

	if (!h)
		return 0;

	return be32(&h->magic) == FDT_MAGIC;
}

uint32_t fdt_size(const void *dtb)
{
	const struct fdt_header *h = dtb;

	if (!fdt_valid(dtb))
		return 0;

	return be32(&h->totalsize);
}

uint32_t fdt_version(const void *dtb)
{
	const struct fdt_header *h = dtb;

	if (!fdt_valid(dtb))
		return 0;

	return be32(&h->version);
}

uint32_t fdt_struct_offset(const void *dtb)
{
	const struct fdt_header *h = dtb;

	if (!fdt_valid(dtb))
		return 0;

	return be32(&h->off_dt_struct);
}

uint32_t fdt_strings_offset(const void *dtb)
{
	const struct fdt_header *h = dtb;

	if (!fdt_valid(dtb))
		return 0;

	return be32(&h->off_dt_strings);
}

int fdt_find_simplefb(const void *dtb, struct tb_simplefb *out)
{
	const struct fdt_header *h = dtb;
	const uint8_t *blob;
	const uint8_t *structure;
	const uint8_t *strings;

	struct fb_node_state nodes[32];

	uint32_t structure_size;
	uint32_t strings_size;
	uint32_t off = 0;

	int depth = -1;

	if (!out || !fdt_valid(dtb))
		return 0;

	clear_fb(out);

	blob = dtb;

	structure = blob + be32(&h->off_dt_struct);
	strings   = blob + be32(&h->off_dt_strings);

	structure_size = be32(&h->size_dt_struct);
	strings_size   = be32(&h->size_dt_strings);

	while (off + 4 <= structure_size) {
		uint32_t token = be32(structure + off);
		off += 4;

		switch (token) {
		case FDT_BEGIN_NODE: {
			uint32_t len = 0;

			depth++;

			if (depth >= 32)
				return 0;

			clear_fb(&nodes[depth].fb);
			nodes[depth].simplefb = 0;

			while ((off + len) < structure_size &&
			       structure[off + len])
				len++;

			if ((off + len) >= structure_size)
				return 0;

			off += align4(len + 1);
			break;
		}

		case FDT_END_NODE:
			if (depth < 0)
				return 0;

			if (nodes[depth].simplefb) {
				struct tb_simplefb *fb = &nodes[depth].fb;

				if (fb->base &&
				    fb->width &&
				    fb->height &&
				    fb->stride) {
					*out = *fb;
					return 1;
				}
			}

			depth--;
			break;

		case FDT_PROP: {
			uint32_t len;
			uint32_t nameoff;
			const char *name;
			const uint8_t *data;

			if (depth < 0 || off + 8 > structure_size)
				return 0;

			len = be32(structure + off);
			nameoff = be32(structure + off + 4);
			off += 8;

			if (nameoff >= strings_size)
				return 0;

			if (off + align4(len) > structure_size)
				return 0;

			name = (const char *)(strings + nameoff);
			data = structure + off;

			if (streq(name, "compatible")) {
				if (compatible_has(data, len,
						   "simple-framebuffer"))
					nodes[depth].simplefb = 1;

			} else if (streq(name, "reg")) {
				parse_reg(&nodes[depth].fb, data, len);

			} else if (streq(name, "width") && len >= 4) {
				nodes[depth].fb.width = be32(data);

			} else if (streq(name, "height") && len >= 4) {
				nodes[depth].fb.height = be32(data);

			} else if (streq(name, "stride") && len >= 4) {
				nodes[depth].fb.stride = be32(data);

			} else if (streq(name, "format")) {
				copy_string(nodes[depth].fb.format,
					    sizeof(nodes[depth].fb.format),
					    data, len);
			}

			off += align4(len);
			break;
		}

		case FDT_NOP:
			break;

		case FDT_END:
			return 0;

		default:
			return 0;
		}
	}

	return 0;
}
