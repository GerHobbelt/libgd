#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>

#include "gd.h"

/* A short program which converts a .gif file into a .gd file, for
	your convenience in creating images on the fly from a
	basis image that must be loaded quickly. The .gd format
	is not intended to be a general-purpose format. */


#if defined(BUILD_MONOLITHIC)
#define main          gd_giftogd2_main
#endif

int
main(int argc, const char** argv)
{
	gdImagePtr im;
	FILE *in, *out;
	int	cs, fmt;

	if (argc != 5) {
		fprintf(stderr, "Usage: giftogd2 filename.gif filename.gd2 cs fmt\n");
		fprintf(stderr, "       where cs is the chunk size\n");
		fprintf(stderr, "	fmt is 1 for raw, 2 for compressed\n");
		return 1;
	}
	in = fopen(argv[1], "rb");
	if (!in) {
		fprintf(stderr, "Input file does not exist!\n");
		return 1;
	}
	im = gdImageCreateFromGif(in);
	fclose(in);
	if (!im) {
		fprintf(stderr, "Input is not in GIF format!\n");
		return 1;
	}
	out = fopen(argv[2], "wb");
	if (!out) {
		fprintf(stderr, "Output file cannot be written to!\n");
		gdImageDestroy(im);
		return 1;
	}
	cs = atoi(argv[3]);
	fmt = atoi(argv[4]);
	gdImageGd2(im, out, cs, fmt);
	fclose(out);
	gdImageDestroy(im);

	return 0;
}
