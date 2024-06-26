#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "gd.h"

/* A short program which converts a .png file into a .gd2 file, for
   your convenience in creating images on the fly from a
   basis image that must be loaded quickly. The .gd2 format
   is not intended to be a general-purpose format. */


#if defined(BUILD_MONOLITHIC)
#define main          gd_gd2topng_main
#endif

int
main(int argc, const char** argv)

{
	gdImagePtr im;
	FILE *in, *out;
	int x=0, y=0, w=0, h=0;
	if ((argc != 3) && (argc != 7)) {
		fprintf (stderr,
		         "Usage: gd2topng filename.gd2 filename.png [srcx srcy width height]\n");
		fprintf (stderr,
		         "If the coordinates are absent, the entire image is converted.\n");
		return 1;
	}
	if (argc == 7) {
		x = atoi (argv[3]);
		y = atoi (argv[4]);
		w = atoi (argv[5]);
		h = atoi (argv[6]);
	}
	in = fopen (argv[1], "rb");
	if (!in) {
		fprintf(stderr, "Input file does not exist!\n");
		return 1;
	}
	if (argc == 7) {
		im = gdImageCreateFromGd2Part (in, x, y, w, h);
	} else {
		im = gdImageCreateFromGd2 (in);
	}
	fclose (in);
	if (!im) {
		fprintf(stderr, "Input is not in GD2 format!\n");
		return 1;
	}
	out = fopen (argv[2], "wb");
	if (!out) {
		fprintf(stderr, "Output file cannot be written to!\n");
		gdImageDestroy (im);
		return 1;
	}
	gdImagePng (im, out);
	fclose (out);
	gdImageDestroy (im);

	return 0;
}
