#ifndef ASPRINTF_H
#define ASPRINTF_H

#if defined(__GNUC__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE /* needed for (v)asprintf, affects '#include <stdio.h>' */
#endif
#include <stdarg.h> /* needed for va_*         */
#include <stdio.h>  /* needed for vsnprintf    */
#include <stdlib.h> /* needed for malloc, free */

#ifdef _MSC_VER
#define vscprintf _vscprintf
#else
int vscprintf(const char *format, va_list ap);
#endif

#ifdef _MSC_VER
int vasprintf(char **strp, const char *format, va_list ap);
int asprintf(char **strp, const char *format, ...);
#endif

#endif // ASPRINTF_H
