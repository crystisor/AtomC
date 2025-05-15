#include "utils.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void err(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "error: ");
    vfprintf(stderr, fmt, va);
    fputc('\n', stderr);
    va_end(va);
    exit(-1);
}


char *createString(const char *start, const char *end)
{
    int len = end - start;
    char *str = (char *)malloc(len + 1);
    if (!str)
        err("not enough memory");
    memcpy(str, start, len);
    str[len] = '\0';
    return str;
}
