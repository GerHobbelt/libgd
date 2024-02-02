#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "gd.h"

/* A short program which converts a .png file into a .gd file, for
   your convenience in creating images on the fly from a
   basis image that must be loaded quickly. The .gd format
   is not intended to be a general-purpose format. */


#if defined(BUILD_MONOLITHIC)
#define main          gd_copypal_main
#endif

int
main(int argc, const char** argv)
{
	gdImagePtr im;
	gdImagePtr pal;
	FILE *in, *out;
	if (argc != 3) {
		fprintf(stderr, "Usage: gd2copypal palettefile.gd2 filename.gd2\n");
		return 1;
	}
	in = fopen (argv[1], "rb");
	if (!in) {
		fprintf(stderr, "Palette file does not exist!\n");
		return 1;
	}
	pal = gdImageCreateFromGd2 (in);
	fclose (in);
	if (!pal) {
		fprintf(stderr, "Palette is not in GD2 format!\n");
		return 1;
	}

	in = fopen (argv[2], "rb");
	if (!in) {
		fprintf(stderr, "Input file does not exist!\n");
		return 1;
	}
	im = gdImageCreateFromGd2 (in);
	fclose (in);
	if (!im) {
		fprintf(stderr, "Input is not in GD2 format!\n");
		return 1;
	}

	gdImagePaletteCopy (im, pal);

	out = fopen (argv[2], "wb");
	if (!out) {
		fprintf(stderr, "Output file cannot be written to!\n");
		gdImageDestroy (im);
		return 1;
	}
	gdImageGd2 (im, out, 128, 2);
	fclose (out);
	gdImageDestroy (pal);
	gdImageDestroy (im);

	return 0;
}
