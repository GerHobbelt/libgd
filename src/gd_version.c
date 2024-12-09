#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "gd.h"


/* These functions return the version information.  We use functions
 * so that changes in the shared library will automatically be
 * reflected in executables using it without recompiling them. */

/*
    Function: gdMajorVersion
*/
BGD_DECLARE(int) gdMajorVersion(void)
{
    return GD_MAJOR_VERSION;
}

/*
    Function: gdMinorVersion
*/
BGD_DECLARE(int) gdMinorVersion(void)
{
    return GD_MINOR_VERSION;
}

/*
    Function: gdReleaseVersion
*/
BGD_DECLARE(int) gdReleaseVersion(void)
{
    return GD_RELEASE_VERSION;
}

/*
    Function: gdExtraVersion
*/
BGD_DECLARE(const char *) gdExtraVersion(void) {
    return GD_EXTRA_VERSION;
}

/*
    Function: gdVersionString
*/
BGD_DECLARE(const char *) gdVersionString(void) {
    return GD_VERSION_STRING;
}
