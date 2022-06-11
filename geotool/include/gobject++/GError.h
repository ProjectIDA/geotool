#ifndef _G_ERROR_H
#define _G_ERROR_H

#define GERROR_MALLOC_ERROR		101
#define GERROR_INVALID_ARGS		102
#define GERROR_SAMPLE_RATE_EXCEPTION	103
#define GERROR_RESPONSE_FILE_ERROR	104

/** An error handler class.
 *  @ingroup libgobject
 */
class GError
{
    public:
#ifdef __STDC__
	static void setMessage(const char *format, ...);
#else
	static void setMessage(va_alist);
#endif
	static char *getMessage(void);

	static void printErrorMessages(FILE *fpout);

    protected:
};

#endif

