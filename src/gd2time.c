#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>		/* for atoi */
#include <time.h>		/* For time */
#include "gd.h"

/* A short program which converts a .png file into a .gd file, for
   your convenience in creating images on the fly from a
   basis image that must be loaded quickly. The .gd format
   is not intended to be a general-purpose format. */


#if defined(BUILD_MONOLITHIC)
#define main          gd_time_main
#endif

int
main(int argc, const char** argv)
{
	gdImagePtr im;
	FILE *in;
	int x, y, w, h;
	int c;
	int i;
	int t0;

	if (argc != 7) {
		fprintf(stderr, "Usage: gd2time filename.gd count x y w h\n");
		return 1;
	}

	c = atoi (argv[2]);
	x = atoi (argv[3]);
	y = atoi (argv[4]);
	w = atoi (argv[5]);
	h = atoi (argv[6]);

	printf ("Extracting %d times from (%d, %d), size is %dx%d\n", c, x, y, w,
	        h);

	t0 = time (0);
	for (i = 0; i < c; i++) {
		in = fopen (argv[1], "rb");
		if (!in) {
			fprintf(stderr, "Input file does not exist!\n");
			return 1;
		}

		im = gdImageCreateFromGd2Part (in, x, y, w, h);
		fclose (in);

		if (!im) {
			fprintf(stderr, "Error reading source file!\n");
			return 1;
		}
		gdImageDestroy (im);
	};
	t0 = time (0) - t0;
	printf ("%d seconds to extract (& destroy) %d times\n", t0, c);

	return 0;
}
