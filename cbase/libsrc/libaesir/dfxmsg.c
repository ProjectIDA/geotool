/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif


#define MSGBUF_MAX	2048

void
#ifdef __STDC__
dfx_warning(char *format, ...)
#else
dfx_warning(va_alist) va_dcl
#endif
{
        va_list va;
        size_t  n;
        char    warning[MSGBUF_MAX];

#ifdef __STDC__
        va_start(va, format);
#else
        char *format = NULL;
        va_start(va);
        format = va_arg(va, char *);
#endif

        if(format == NULL || (n = (int)strlen(format)) <= 0) return;

        vsnprintf(warning, MSGBUF_MAX, format, va);
        va_end(va);

	fprintf(stderr, "warning: %s\n", warning);


}


void
#ifdef __STDC__
dfx_exit(char *format, ...)
#else
dfx_exit(va_alist) va_dcl
#endif
{
        va_list va;
        size_t  n;
        char    warning[MSGBUF_MAX];

#ifdef __STDC__
        va_start(va, format);
#else
        char *format = NULL;
        va_start(va);
        format = va_arg(va, char *);
#endif

        if(format == NULL || (n = (int)strlen(format)) <= 0) return;

        vsnprintf(warning, MSGBUF_MAX, format, va);
        va_end(va);

	fprintf(stderr, "error: %s\n", warning);


}



