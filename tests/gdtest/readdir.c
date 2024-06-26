#include <malloc.h>
#include <string.h>
#include <errno.h>
#include "readdir.h"

/**********************************************************************
 * Implement dirent-style opendir/readdir/rewinddir/closedir on Win32
 *
 * Functions defined are opendir(), readdir(), rewinddir() and
 * closedir() with the same prototypes as the normal dirent.h
 * implementation.
 *
 * Does not implement telldir(), seekdir(), or scandir().  The dirent
 * struct is compatible with Unix, except that d_ino is always 1 and
 * d_off is made up as we go along.
 *
 * The DIR typedef is not compatible with Unix.
 **********************************************************************/

DIR *opendir(const char *path)
{
	DIR *dp;
	char *filespec;
	HANDLE handle;
	int index;

	if (path == NULL) {
		return NULL;
	}

	filespec = (char *)malloc(strlen(path) + 2 + 1);
	if (filespec == NULL) {
		return NULL;
	}
	strcpy(filespec, path);
	index = (int)strlen(filespec) - 1;
	if (index >= 0 && (filespec[index] == '/' ||
	   (filespec[index] == '\\' && (index == 0 || !IsDBCSLeadByte(filespec[index-1])))))
		filespec[index] = '\0';
	strcat(filespec, "\\*");

	dp = (DIR *) malloc(sizeof(DIR));
	if (dp == NULL) {
		free(filespec);
		return NULL;
	}
	dp->offset = 0;
	dp->finished = 0;

	if ((handle = FindFirstFileA(filespec, &(dp->fileinfo))) == INVALID_HANDLE_VALUE) {
		DWORD err = GetLastError();
		if (err == ERROR_NO_MORE_FILES || err == ERROR_FILE_NOT_FOUND) {
			dp->finished = 1;
		} else {
			free(dp);
			free(filespec);
			return NULL;
		}
	}
	dp->dir = strdup(path);
	dp->handle = handle;
	free(filespec);

	return dp;
}

size_t gd_strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

struct dirent *readdir(DIR *dp)
{
	if (!dp || dp->finished)
		return NULL;

	if (dp->offset != 0) {
		if (FindNextFileA(dp->handle, &(dp->fileinfo)) == 0) {
			dp->finished = 1;
			return NULL;
		}
	}
	dp->offset++;

	gd_strlcpy(dp->dent.d_name, dp->fileinfo.cFileName, _MAX_FNAME+1);
	dp->dent.d_ino = 1;
	dp->dent.d_reclen = (unsigned short)strlen(dp->dent.d_name);
	dp->dent.d_off = dp->offset;

	return &(dp->dent);
}

int readdir_r(DIR *dp, struct dirent *entry, struct dirent **result)
{
	if (!dp || dp->finished) {
		*result = NULL;
		return 0;
	}

	if (dp->offset != 0) {
		if (FindNextFileA(dp->handle, &(dp->fileinfo)) == 0) {
			dp->finished = 1;
			*result = NULL;
			return 0;
		}
	}
	dp->offset++;

	gd_strlcpy(dp->dent.d_name, dp->fileinfo.cFileName, _MAX_FNAME+1);
	dp->dent.d_ino = 1;
	dp->dent.d_reclen = (unsigned short)strlen(dp->dent.d_name);
	dp->dent.d_off = dp->offset;

	memcpy(entry, &dp->dent, sizeof(*entry));

	*result = &dp->dent;

	return 0;
}

int closedir(DIR *dp)
{
	if (!dp)
		return 0;
	/* It is valid to scan an empty directory but we have an invalid
	   handle in this case (no first file found). */
	if (dp->handle != INVALID_HANDLE_VALUE) {
		FindClose(dp->handle);
	}
	if (dp->dir)
		free(dp->dir);
	if (dp)
		free(dp);

	return 0;
}

int rewinddir(DIR *dp)
{
	/* Re-set to the beginning */
	char *filespec;
	HANDLE handle;
	int index;

	FindClose(dp->handle);

	dp->offset = 0;
	dp->finished = 0;

	filespec = (char *)malloc(strlen(dp->dir) + 2 + 1);
	if (filespec == NULL) {
		return -1;
	}

	strcpy(filespec, dp->dir);
	index = (int)strlen(filespec) - 1;
	if (index >= 0 && (filespec[index] == '/' ||
	   (filespec[index] == '\\' && (index == 0 || !IsDBCSLeadByte(filespec[index-1])))))
		filespec[index] = '\0';
	strcat(filespec, "/*");

	if ((handle = FindFirstFileA(filespec, &(dp->fileinfo))) == INVALID_HANDLE_VALUE) {
		dp->finished = 1;
	}

	dp->handle = handle;
	free(filespec);

	return 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: sw=4 ts=4 fdm=marker
 * vim<600: sw=4 ts=4
 */
