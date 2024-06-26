#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gd.h"

#if !defined(_WIN32) || defined(BUILD_MONOLITHIC)

// getline: when we don't have it in the RTL, we grab it from libmagic, i.e. GNU file's portability code.
#ifndef HAVE_GETLINE
#include <file.h>
#endif


/* A neat little utility which adds freetype text to
 * existing JPEG images. Type annotate -h for instructions.
 * Thanks to Joel Dubiner for supporting this work. -TBB
 */

enum { left, center, right };


#if defined(BUILD_MONOLITHIC)
#define main          gd_annotate_main
#endif

int
main(int argc, const char** argv)
{
	gdImagePtr im;
	const char *iin, *iout;
	FILE *in, *out;
	char *s;
	size_t len;
	int bounds[8];
	int lines = 1;
	int color = gdTrueColor(0, 0, 0);
	char *font;
	int size = 12;
	int align = left;
	int x = 0, y = 0;
	char *fontError;

	font = strdup("times");

	if(argc != 3) {
		fprintf(stderr, "Usage: annotate imagein.jpg imageout.jpg\n\n");
		fprintf(stderr, "Standard input should consist of\n");
		fprintf(stderr, "lines in the following formats:\n");
		fprintf(stderr, "color r g b (0-255 each) [a (0-127, 0 is opaque)]\n");
		fprintf(stderr, "font fontname\n");
		fprintf(stderr, "size pointsize\n");
		fprintf(stderr, "align (left|right|center)\n");
		fprintf(stderr, "move x y\n");
		fprintf(stderr, "text actual-output-text\n\n");
		fprintf(stderr,
		        "If the file 'paris.ttf' exists in /usr/share/fonts/truetype or in a\n");
		fprintf(stderr,
		        "location specified in the GDFONTPATH environment variable, 'font paris' is\n");
		fprintf(stderr,
		        "sufficient. You may also specify the full, rooted path of a font file.\n");
		return 1;
	}

	iin = argv[1];
	iout = argv[2];

	in = fopen(iin, "rb");
	if(!in) {
		fprintf(stderr, "Couldn't open %s\n", iin);
		return 2;
	}

	im = gdImageCreateFromJpeg(in);

	fclose(in);

	if(!im) {
		fprintf(stderr, "%s did not load properly\n", iin);
		return 3;
	}

	s = NULL;
	len = 0;
	while (getline(&s, &len, stdin) != -1) {
		char *st;
		char *text;

		st = strtok(s, " \t\r\n");
		if(!st) {
			/* Be nice about blank lines */
			continue;
		}

		if(!strcmp(st, "font")) {
			char *st = strtok(0, " \t\r\n");
			if(!st) {
				goto badLine;
			} else {
				free(font);
				font = strdup(st);
				if (font == NULL) {
					perror("Font failed");
					goto badLine;
				}
			}
		} else if(!strcmp(st, "align")) {
			char *st = strtok(0, " \t\r\n");

			if(!st) {
				goto badLine;
			}

			if(!strcmp(st, "left")) {
				align = 0;
			} else if(!strcmp(st, "center")) {
				align = 1;
			} else if(!strcmp(st, "right")) {
				align = 2;
			}
		} else if(!strcmp(st, "size")) {
			char *st = strtok(0, " \t\r\n");

			if(!st) {
				goto badLine;
			}

			size = atoi(st);
		} else if(!strcmp(st, "color")) {
			char *st = strtok(0, "\r\n");
			int r, g, b, a = 0;

			if(!st) {
				goto badLine;
			}

			if(sscanf(st, "%d %d %d %d", &r, &g, &b, &a) < 3) {
				fprintf(stderr, "Bad color at line %d\n", lines);
				return 2;
			}

			color = gdTrueColorAlpha(r, g, b, a);
		} else if(!strcmp(st, "move")) {
			char *st = strtok(0, "\r\n");

			if(!st) {
				goto badLine;
			}

			if(sscanf(st, "%d %d", &x, &y) != 2) {
				fprintf(stderr, "Missing coordinates at line %d\n", lines);
				return 3;
			}
		} else if(!strcmp(st, "text")) {
			int rx = x;

			text = strtok(0, "\r\n");
			if(!text) {
				text = "";
			}

			gdImageStringFT(0, bounds, color, font, size, 0, x, y, text);

			switch(align) {
			case left:
				break;

			case center:
				rx -= (bounds[2] - bounds[0]) / 2;
				break;

			case right:
				rx -= (bounds[2] - bounds[0]);
				break;
			}

			fontError = gdImageStringFT(im, 0, color, font, size, 0, rx, y, text);
			if(fontError) {
				fprintf(stderr, "Font error at line %d: %s\n", lines, fontError);
				return 7;
			}

			y -= (bounds[7] - bounds[1]);
		} else {
			goto badLine;
		}

		lines++;
		continue;

badLine:
		fprintf(stderr, "Bad syntax, line %d\n", lines);
		return 4;
	}
	free(font);
	free(s);

	out = fopen(iout, "wb");
	if(!out) {
		fprintf(stderr, "Cannot create %s\n", iout);
		return 5;
	}
	gdImageJpeg(im, out, 95);
	gdImageDestroy(im);
	fclose(out);
	return 0;
}

#endif
