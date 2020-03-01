// SPDX-License-Identifier: GPL-3.0-only

#include "file.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH_MIN	8
#define WIDTH_MAX	4096
#define BIT(n)		(1 << (n))
#define UNUSED(v)	((void)v)

enum mode {
	MODE_HORIZONTAL,
	MODE_VERTICAL,
};

struct params {
	char *fpath;	/* image file path; NULL in case of stdin */
	size_t width;	/* image width */
	enum mode mode;	/* SSD1306 memory addressing mode */
};

typedef void (*map_func_t)(size_t in_byte, size_t in_bit, size_t width,
			   size_t height, char *out);

static void print_usage(const char *app)
{
	printf("Usage:\n"
	       "  %s <mono-image> <width> {v|h}\n"
	       "  cat <mono-image> | %s <width> {v|h}\n\n"
	       "Convert image for OLED usage. SSD1306 controller expects\n"
	       "data to be arranged in one of two very specific ways:\n"
	       "  - horizontal: 1 byte is 8 vertical bits, and next byte will\n"
	       "                be on the right\n"
	       "  - vertical: 1 byte is 8 vertical bits, and next byte will\n"
	       "              be on the bottom\n\n"
	       "This tool takes monochrome bitmap (not BMP, just raw binary!)\n"
	       "in regular row-major order (where each byte represents 8\n"
	       "horizontal bits in Little Endian, and each next byte is on\n"
	       "the right), and transforms it to the bitmap for SSD1306 OLED,\n"
	       "using specified addressing mode (h or v). Output file also\n"
	       "has Little Endian byte order.\n\n"
	       "Options:\n"
	       "  <mono-image>  binary bitmap representation of image\n"
	       "  <width>       image width\n"
	       "  {v|h}         vertical or horizontal addressing\n\n"
	       "Examples:\n\n"
	       "  convert image.png mono:- | %s 128 h | xxd -i\n\n"
	       "  convert image.png image.mono\n"
	       "  %s image.mono 128 v > image.lcd\n"
	       "  xxd -i image.lcd image.c\n",
	       app, app, app, app);
}

static bool parse_args(struct params *p, int argc, char **argv)
{
	int width;

	if (argc == 2 && !strcmp(argv[1], "--help")) {
		print_usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	--argc;
	++argv;

	if (argc > 3 || argc < 2) {
		fprintf(stderr, "Error: Invalid arguments count\n");
		return false;
	}

	if (argc == 3) {
		p->fpath = argv[0];
		--argc;
		++argv;
	} else {
		p->fpath = NULL;
	}

	errno = 0;
	width = strtol(argv[0], NULL, 10);
	if (errno) {
		perror("Error: Wrong width format");
		return false;
	}
	p->width = width;

	if (strlen(argv[1]) > 1) {
		fprintf(stderr, "Error: Wrong mode param\n");
		return false;
	}

	switch (argv[1][0]) {
	case 'h':
		p->mode = MODE_HORIZONTAL;
		break;
	case 'v':
		p->mode = MODE_VERTICAL;
		break;
	default:
		fprintf(stderr, "Error: Wrong mode param\n");
		return false;
	}

	return true;
}

static bool validate_args(struct params *p)
{
	if (p->fpath && !file_exist(p->fpath)) {
		perror("Error: File doesn't exist");
		return false;
	}

	if (p->width < WIDTH_MIN || p->width > WIDTH_MAX) {
		fprintf(stderr, "Error: Wrong width value %zu\n", p->width);
		return false;
	}

	return true;
}

static void map_horizontal(size_t in_byte, size_t in_bit, size_t width,
			   size_t height, char *out)
{
	size_t row = (in_byte * 8) / width;
	size_t col = (in_byte * 8) % width + in_bit;
	size_t page = row / 8;
	size_t page_off = width * 8 * page;
	size_t row_in_page = row % 8;
	size_t bit = page_off + 8 * col + row_in_page;
	size_t out_byte = bit / 8;
	size_t out_bit = bit % 8;

	UNUSED(height);

	out[out_byte] |= BIT(out_bit);
}

static void map_vertical(size_t in_byte, size_t in_bit, size_t width,
			 size_t height, char *out)
{
	size_t bytes_per_row = width / 8;
	size_t byte_in_row = in_byte % bytes_per_row;
	size_t row = in_byte / bytes_per_row;
	size_t col = byte_in_row * 8 + in_bit;
	size_t pages = height / 8;
	size_t page = row / 8;
	size_t out_byte = pages * col + page;
	size_t out_bit = row % 8;

	out[out_byte] |= BIT(out_bit);
}

/**
 * Transform 2D array from row-major order to OLED format.
 *
 * Caller must free array allocated in this function.
 *
 * @param in Array in row-major order (regular C array)
 * @param len Array elements count
 * @param width Columns count in 2D array
 * @param map Map function to use
 * @return Array in OLED format
 */
static char *arr2oled(const char *in, size_t len, size_t width, size_t height,
		      map_func_t map)
{
	char *out = calloc(len, 1);
	size_t i, j;

	for (i = 0; i < len; ++i) {
		if (in[i] == 0x00)
			continue;

		for (j = 0; j < 8; ++j) {
			if ((in[i] & BIT(j)))
				map(i, j, width, height, out);
		}
	}

	return out;
}

int main(int argc, char *argv[])
{
	struct params p;
	char *arr_orig, *arr_trans;
	map_func_t map;
	size_t len, wlen, height;
	int ret = EXIT_SUCCESS;

	if (!parse_args(&p, argc, argv)) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (!validate_args(&p)) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	arr_orig = file_read(p.fpath, &len);
	if (!arr_orig)
		return EXIT_FAILURE;

	height = (len * 8) / p.width;
	assert(((len * 8) % p.width) == 0);
	assert((height % 8) == 0);
	assert(height >= 8);

	map = p.mode == MODE_HORIZONTAL ? map_horizontal : map_vertical;
	arr_trans = arr2oled(arr_orig, len, p.width, height, map);

	wlen = fwrite(arr_trans, 1, len, stdout);
	if (wlen != len) {
		fprintf(stderr, "Error: Unable to write the whole file\n");
		ret = EXIT_FAILURE;
	}

	free(arr_trans);
	free(arr_orig);
	return ret;
}
