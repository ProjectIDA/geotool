/*	SccsId:	%W%	%G%	*/

#ifndef _LIBSTRING_H_
#define	_LIBSTRING_H_

#include <stdio.h>
#include <string.h>

/* ****** string.c ********/
char *	stringToUpper(char *str);
char *	stringToLower(char *str);
char *	stringReplace(char *str, const char *old_pattern,
			const char *new_pattern);
char *	stringTrim(char *c);
int	stringToBool(const char *text, int *b);
int	stringToDouble(const char *text, double *d);
int	stringToLong(const char *text, long *l);
int	stringToInt(const char *text, int *i);
int	stringToShort(const char *text, short *s);
int	stringToUShort(const char *text, unsigned short *s);
char *	stringCleanPath(char *path);
char *	stringGetArg(const char *str, const char *name);
int	stringGetDoubleArg(const char *str, const char *name, double *value);
int	stringGetIntArg(const char *str, const char *name, int *value);
int	stringGetLongArg(const char *str, const char *name, long *value);
int	stringGetBoolArg(const char *str, const char *name, int *value);
char *	stringCopy(const char *str);
char *	stringNCopy(const char *s, int n);
char *	stringGetArgv(int argc, const char **argv, const char *name);
int	stringGetDoubleArgv(int argc, const char **argv, const char *name,
			double *value);
int	stringGetLongArgv(int argc, const char **argv, const char *name,
			long *value);
int	stringGetIntArgv(int argc, const char **argv, const char *name,
			int *value);
int	stringGetBoolArgv(int argc, const char **argv, const char *name,
			int *value);
int	stringGetLine(FILE *fp, char *line, int n);
int	stringGetSuffix(const char *filename);
const char *stringSuffix(const char *filename);
int	stringGetPrefix(const char *filename);
int	stringGetDir(const char *filename);
char *	stringTrimCopy(char *a, const char *b, int n);
int	stringTrimToQuark(const char *a);
int	stringEndsWith(const char *s, const char *ending);
int	stringCaseEndsWith(const char *s, const char *ending);
char *	stringcpy(char *dest, const char *src, int n);
int	stringCaseMatch(const char *a, const char *b);
int	stringMatch(const char *a, const char *b);
char *  stringBaseName (const char * pathName);
int	stringUpperToQuark(const char *s);
int   stringArg(const char *s, const char *name, char **value);

/* ****** quark.c ********/
int	stringToQuark(const char *name);
int	stringNToQuark(const char *name, int len);
const char *quarkToString(int quark);

/* ****** strtok_r.c ********/
#ifndef HAVE_STRTOK_R
char *	strtok_r(char *str, const char *tok, char **last);
#endif /* HAVE_STRTOK_R */


/* ****** parse_char.c ********/
int check_query_char(const char *clause, char *new_clause, int len,
			const char *type, int lower_case);
int check_query_int(const char *clause, char *new_clause, int len,
			const char *type);


/* ****** checks.c ********/
int stringToLatitude(const char *str, double *val);
int stringToLongitude(const char *str, double *val);
int stringToDepth(const char *str, double *val);
int stringIsNumber(const char *str);
int stringIsOnlySpaces(const char *str);
int stringToDistanceDeg(const char *str, double *val);
int stringToDistanceKm(const char *str, double *val);
int stringToAzimuth(const char *str, double *val);


/* ******* MallocWarn.c ********/

void *mallocWarn(int nbytes);
void *reallocWarn(void *ptr, int nbytes);


#endif /* _LIBSTRING_H_ */
