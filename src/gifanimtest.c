#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gd.h"


#if defined(BUILD_MONOLITHIC)
#define main          gd_gifanimtest_main
#endif

int main(void)
{
	gdImagePtr im;
	gdImagePtr last, cim;
	int blue;
	FILE *out;
	int i;
	im = gdImageCreate(100, 100);
	blue = gdImageColorAllocate(im, 0, 0, 255);
	out = fopen("gifanim.gif", "wb");
	gdImageGifAnimBegin(im, out, 1, 10);
	last = 0;
	for (i = 0; (i <= 100); i += 2) {
		cim = gdImageCreate(100, 100);
		gdImagePaletteCopy(cim, im);
		gdImageArc(cim, 50, 50, i, i, 0, 360, blue);
		gdImageGifAnimAdd(cim, out, 0, 0, 0, 10, gdDisposalNone, last);
		if (last) {
			gdImageDestroy(last);
		}
		last = cim;
	}
	gdImageDestroy(last);
	gdImageGifAnimEnd(out);
	fclose(out);
	gdImageDestroy(im);
	return 0;
}
