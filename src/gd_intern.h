/* Internal header for random common utility functions. */

#ifndef GD_INTERN_H
#define GD_INTERN_H

#include <limits.h>

#ifndef MAXPATHLEN
# ifdef PATH_MAX
#  define MAXPATHLEN PATH_MAX
# elif defined(MAX_PATH)
#  define MAXPATHLEN MAX_PATH
# else
#  if defined(__GNU__)
#   define MAXPATHLEN 4096
#  else
#   define MAXPATHLEN 256    /* Should be safe for any weird systems that do not define it */
#  endif
# endif
#endif

#ifdef HAVE_STDINT_H
# include <stdint.h>
#else
# if defined(HAVE_INTTYPES_H)
#  include <inttypes.h>
# else
#  include "msinttypes/inttypes.h"
# endif
#endif

#ifdef _MSC_VER
#define  ssize_t SSIZE_T
#define SSIZE_MAX MAXSSIZE_T
#endif

#include "gd.h"

#define MIN(a,b) ((a)<(b)?(a):(b))
#define MIN3(a,b,c) ((a)<(b)?(MIN(a,c)):(MIN(b,c)))
#define MAX(a,b) ((a)<(b)?(b):(a))
#define MAX3(a,b,c) ((a)<(b)?(MAX(b,c)):(MAX(a,c)))
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

typedef enum {
    HORIZONTAL,
    VERTICAL,
} gdAxis;

/* Convert a double to an unsigned char, rounding to the nearest
 * integer and clamping the result between 0 and max.  The absolute
 * value of clr must be less than the maximum value of an unsigned
 * short. */
static inline unsigned char
uchar_clamp(double clr, unsigned char max) {
	unsigned short result;

	//assert(fabs(clr) <= SHRT_MAX);

	/* Casting a negative float to an unsigned short is undefined.
	 * However, casting a float to a signed truncates toward zero and
	 * casting a negative signed value to an unsigned of the same size
	 * results in a bit-identical value (assuming twos-complement
	 * arithmetic).	 This is what we want: all legal negative values
	 * for clr will be greater than 255. */

	/* Convert and clamp. */
	result = (unsigned short)(short)(clr + 0.5);
	if (result > max) {
		result = (clr < 0) ? 0 : max;
	}/* if */

	return result;
}/* uchar_clamp*/


/* Internal prototypes: */

/* gd_rotate.c */
gdImagePtr gdImageRotate90(gdImagePtr src, int ignoretransparent);
gdImagePtr gdImageRotate180(gdImagePtr src, int ignoretransparent);
gdImagePtr gdImageRotate270(gdImagePtr src, int ignoretransparent);






#endif
