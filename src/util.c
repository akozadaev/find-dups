/*
 *  util.c
 *  Author: Alex Kozadaev (2015)
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"

void
die(const char *errstr, ...)
{
    va_list ap;

    va_start(ap, errstr);
    vfprintf(stderr, errstr, ap);
    va_end(ap);
    exit(1);
}

inline char
*xstrndup(const char *src, size_t max)
{
    char *dst;
    int n = strlen(src) + 1;

    if (n > max) {
        n = max;
    }

    dst = malloc(n);
    if (!dst) {
        die("xstrndup: out of memory\n");
    }
    strncpy(dst, src, n);
    dst[n - 1] = '\0';

    return dst;
}

