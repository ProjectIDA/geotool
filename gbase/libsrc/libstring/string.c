#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "libstring.h"
#include "logErrorMsg.h"

#ifndef True
#define True 1
#endif
#ifndef False
#define False 0
#endif

/**
 * Utility routines for character strings.
 */

/**
 * Convert all lower case characters to upper case.
 * @param str A null terminated string. All lower case characters in str are \
 *	be converted to upper case.
 * @return str
 */
char *
stringToUpper(char *str)
{
	int i;

	for(i = 0; i < (int)strlen(str); i++)
        {
	    str[i] = toupper((int)str[i]);
        }
        return str;
}

/**
 * Convert all upper case characters to lower case.
 * @param str A null terminated string. All upper case characters in str are \
 *	be converted to lower case.
 * @return str
 */
char *
stringToLower(char *str)
{
	int i;

	for(i = 0; i < (int)strlen(str); i++)
	{
	    str[i] = tolower((int)str[i]);
	}
	return str;
}

/**
 * Replace a pattern.  In the string str, replace all occurrences of
 * old_pattern with new_pattern. The length of new_pattern should be <= the
 * length of old_pattern. If the length of new_pattern is greater than the
 * length of old_pattern, only characters in new_pattern up to the smaller
 * length will be used.
 * @param str The character string that will be modified.
 * @param old_pattern The substring that will be replaced in str.
 * @param new_pattern The replacement substring.
 * @return str
 */
char *
stringReplace(char *str, const char *old_pattern, const char *new_pattern)
{
        char *c, *p, *s;
	int oldlen, newlen;

	if(str == NULL || old_pattern == NULL || new_pattern == NULL) {
	    return str;
	}
	oldlen = strlen(old_pattern);
	newlen = strlen(new_pattern);
	if(newlen > oldlen) {
	    newlen = oldlen;
	}

	s = str;
	while((p = strstr(s, old_pattern)) != NULL)
        {
	    s += newlen;
	    strncpy(p, new_pattern, newlen);
	    c = p + oldlen;
	    p += newlen;
	    while(*c != '\0') {
		*p = *c;
		c++;
		p++;
	    }
	    *p = '\0';
	}
	return str;
}

/**
 * Remove white space before and after the text. Remove all white-space
 * characters from the beginning and from the end of the input string.
 * The non-white-space characters are shifted to the beginning of the string.
 * White-spaces are space, form-feed ('\f'), newline ('\n'),
 * carriage return ('\r'), horizontal tab ('\t'), and vertical tab ('\v').
 * @param str The character string to be modified.
 * @return str
 */
char *
stringTrim(char *str)
{
    int i, j, k, n;

    n = (int)strlen(str);
    for(j = n-1; j >= 0 && isspace((int)str[j]); j--) n--;
    if(n > 0) {
	for(i = 0; i < n && isspace((int)str[i]); i++);
	if(i > 0) for(k = i; k < n; k++) str[k-i] = str[k];
	n -= i;
    }
    if(str[n] != '\0') str[n] = '\0';
    return str;
}

/**
 * Convert a string to boolean (0 or 1). If the string is (ignoring case)
 * "t", "true", "1", "f", "false", or "0", the conversion is successful.
 * Otherwise not. Set *b to 1 if text is "t", "true" or "1". Set *b to 0 if
 * text is "f", "false", or "0". Case is ignored.
 * @param text The input string.
 * @param b Set to 0 or 1, if the conversion is successful.
 * @return 1 if the conversion is successful, 0 otherwise.
 */
int
stringToBool(const char *text, int *b)
{
    const char *c;
    int n;

    if(text == NULL) return False;

    for(c = text; *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c == '\0') {
        return False;
    }

    if(!strncasecmp(c, "t", n) || !strncasecmp(c, "true", n) ||
		!strncmp(c, "1", n)) {
	*b = 1;
	return True;
    }
    else if(!strncasecmp(c, "f", n) || !strncasecmp(c, "false", n) ||
		!strncmp(c, "0", n)) {
	*b = 0;
	return True;
    }
    return False;
}

/**
 * Convert a string to a double.
 * @param text The input string.
 * @param d Set to the double value.
 * @return 1 if the conversion is successful, 0 otherwise.
 */
int
stringToDouble(const char *text, double *d)
{
    char *endptr, last_char;
    const char *c;
    int n;

    if(text == NULL) return False;

    for(c = text; *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c == '\0') {
        return False;
    }

    *d = strtod(c, &endptr);
    last_char = *endptr;
    return (last_char == c[n]) ? True : False;
}

/**
 * Convert a string to a long.
 * @param text The input string.
 * @param l Set to the long value.
 * @return 1 if the conversion is successful, 0 otherwise.
 */
int
stringToLong(const char *text, long *l)
{
    char *endptr, last_char;
    const char *c;
    int n;

    if(text == NULL) return False;

    for(c = text; *c != '\0' && isspace((int)(*c)); c++);
    n = (int)strlen(c);
    while(n > 0 && isspace((int)c[n-1])) n--;

    if(*c == '\0') {
        return False;
    }

    *l = strtol(c, &endptr, 10);
    last_char = *endptr;
    return (last_char == c[n]) ? True : False;
}

/**
 * Convert a string to a int.
 * @param text The input string.
 * @param i Set to the int value.
 * @return 1 if the conversion is successful, 0 otherwise.
 */
int
stringToInt(const char *text, int *i)
{
    long l;

    if(stringToLong(text, &l)) {
	*i = (int)l;
	return True;
    }
    return False;
}

/**
 * Convert a string to a short.
 * @param text The input string.
 * @param s Set to the long value.
 * @return 1 if the conversion is successful, 0 otherwise.
 */
int
stringToShort(const char *text, short *s)
{
    long l;

    if(stringToLong(text, &l)) {
	*s = (short)l;
	return True;
    }
    return False;
}

/**
 * Convert a string to an unsigned short.
 * @param text The input string.
 * @param s Set to the long value.
 * @return 1 if the conversion is successful, 0 otherwise.
 */
int
stringToUShort(const char *text, unsigned short *s)
{
    long l;

    if(stringToLong(text, &l)) {
	*s = (unsigned short)l;
	return True;
    }
    return False;
}

/**
 * Remove extra directory separators from a file path.  Remove "/../" or
 * "/./" from a path name (like realpath on SUN).
 * @param path The file path to be modified.
 * @return path
 */
char *
stringCleanPath(char *path)
{
	char *c, *d, *p;

	p = path;
	while((c = strstr(p, "//")) != NULL)
	{
	    c++;
	    while(*(c+1) != '\0') { *c = *(c+1); c++; }
	    *c = '\0';
	    p += 2;
	}
	p = path;
	while((c = strstr(p, "/./")) != NULL)
	{
	    c++;
	    while(*(c+2) != '\0') { *c = *(c+2); c++; }
	    *c = '\0';
	    p += 3;
	}
	p = path;
	while((c = strstr(p, "/-/")) != NULL)
	{
	    c++;
	    while(*(c+2) != '\0') { *c = *(c+2); c++; }
	    *c = '\0';
	    p += 3;
	}
	p = path;
	while((c = strstr(p, "/../")) != NULL)
	{
	    for(d = c-1; d > path && *d != '/'; d--);
	    d++;
	    p = d;
	    while(*(c+4) != '\0') { *d = *(c+4); d++; c++;}
	    *d = '\0';
	}
	return path;
}

#define isSpace(c) (isspace((int)(c)) || c == '=')

/**
 * Return the string parameter value. Search the input string for the first
 * occurrence of name=value and return value.
 * @param str The input string to search.
 * @param name The parameter name.
 * @return The string value, or NULL if name is not found. The returned pointer
 * should be freed after use.
 */
char *
stringGetArg(const char *str, const char *name)
{
	int n;
	char *beg, *end, *copy, quote;
	const char *look;

	if(!str || (n = (int)strlen(name)) <= 0) return NULL;

	/* find name offset with white space or '='
	 */
	look = str;
	while((beg = strstr(look, name)) != NULL)
        {
	    if( (     beg == str  || isSpace(*(beg-1))) &&
		(*(beg+n) == '\0' || isSpace(*(beg+n))) ) break;
	    look = beg+n;
	}
	if(beg == NULL) return NULL;

	/* get the token following name
	 */
	beg += n;
	while(*beg != '\0' && isSpace(*beg)) beg++;

	if(*beg == '"' || *beg == '\'' || *beg == '`')
	{
	    /* token is enclosed in quotes */
	    quote = *beg;
	    beg++;
	    end = beg;
	    while(*end != '\0' && *end != quote) end++;
	}
        else if(*beg == '(')    /* token is enclosed in () */
        {
	    int num = 1;
//	    beg++;	// return '(' with the string
	    end = beg;
	    while(num > 0) {    /* takes care of embedded '('s */
		while(*end != '\0' && *end != ')') {
		    if(*end == '(') num++;
		    end++;
		}
		num--;
//		if(num) end++; 
		end++;	// return ')' with the string
	    }
	}
        else if(*beg == '[')    /* token is enclosed in [] */
        {
	    int num = 1;
//	    beg++;	// return '(' with the string
	    end = beg;
	    while(num > 0) {    /* takes care of embedded '['s */
		while(*end != '\0' && *end != ']') {
		    if(*end == '[') num++;
		    end++;
		}
		num--;
//		if(num) end++;
		end++;	// return ']' with the string
	    }
	}
	else
	{
	    end = beg;
	    while(*end != '\0' && !isSpace(*end)) end++;
	}

	copy = stringNCopy(beg, end-beg);

	return copy;
}

/**
 * Return the double parameter value. Search the input string for the first
 * occurrence of name=value and return value as a double.
 * @param str The input string to search.
 * @param name The parameter name.
 * @return 0 for success, -1 if the name was not found, -2 if name was found \
 *	but the value is not a double.
 */
int
stringGetDoubleArg(const char *str, const char *name, double *value)
{
	char *arg;

	if(str && (arg = stringGetArg(str, name)) != NULL)
	{
	    int err = stringToDouble(arg, value);
	    free(arg);
	    return err ? 0 : -2;
	}
	return -1;
}

/**
 * Return the int parameter value. Search the input string for the first
 * occurrence of name=value and return value as an int.
 * @param str The input string to search.
 * @param name The parameter name.
 * @return 0 for success, -1 if the name was not found, -2 if name was found \
 *	but the value is not an integer.
 */
int
stringGetIntArg(const char *str, const char *name, int *value)
{
	char *arg;

	if(str && (arg = stringGetArg(str, name)) != NULL)
	{
	    int err = stringToInt(arg, value);
	    free(arg);
	    return err ? 0 : -2;
	}
	return -1;
}

/**
 * Return the long parameter value. Search the input string for the first
 * occurrence of name=value and return value as a long.
 * @param str The input string to search.
 * @param name The parameter name.
 * @return 0 for success, -1 if the name was not found, -2 if name was found \
 *	but the value is not a long.
 */
int
stringGetLongArg(const char *str, const char *name, long *value)
{
	char *arg;

	if(str && (arg = stringGetArg(str, name)) != NULL)
	{
	    int err = stringToLong(arg, value);
	    free(arg);
	    return err ? 0 : -2;
	}
	return -1;
}

/**
 * Return the boolean parameter value. Search the input string for the first
 * occurrence of name=value and return value as a boolean (0 or 1).
 * @param str The input string to search.
 * @param name The parameter name.
 * @return 0 for success, -1 if the name was not found, -2 if name was found \
 *	but the value is not a long.
 * @see stringToBool
 */
int
stringGetBoolArg(const char *str, const char *name, int *value)
{
	char *arg;

	if(str && (arg = stringGetArg(str, name)) != NULL)
	{
	    int err = stringToBool(arg, value);
	    free(arg);
	    return err ? 0 : -2;
	}
	return -1;
}

/**
 * Create a copy of a string. Return a pointer to allocated memory that contains
 * a copy of the input string.
 * @param s The character string to be copied.
 * @return A pointer to the copied string. Free it after use. Returns NULL, if \
 * the memory allocation failed.
 */
char *
stringCopy(const char *s)
{
	char *copy;
	int len;

	if(s == NULL) {
	    s = "";
	}
	len = strlen(s)+1;
	if((copy = (char *)malloc(len)) == NULL) {
	    logErrorMsg(LOG_ERR, "stringCopy: malloc failed.");
	    return NULL;
	}
	strncpy(copy, s, len);
	return copy;
}

/**
 * Create a copy of a substring. Return a pointer to allocated memory that
 * contains a copy of the input substring.
 * @param s The character string to be copied.
 * @param n The number of characters of string s to copy. If n > strlen(s), \
 * the returned string will be padded to length n with the null character.
 * @return A pointer to the copied string. Free it after use. Returns NULL, if \
 * the memory allocation failed.
 */
char *
stringNCopy(const char *s, int n)
{
	char *copy;
	int len;

	len = (n > 0) ? n+1 : 1;
	if((copy = (char *)malloc(len)) == NULL) {
	    logErrorMsg(LOG_ERR, "stringCopy: malloc failed.");
	    return NULL;
	}
	if(s == NULL || n <= 0) {
	    copy[0] = '\0';
	}
	else {
	    int i, m = strlen(s);
	    strncpy(copy, s, n);
	    for(i = m; i < n; i++) copy[i] = '\0';
	    copy[n] = '\0';
	}
	return copy;
}

/**
 * Copy b to a, skipping spaces at the beginning and end of the string b.
 * @param a String of length >= n (included '\0') that will receive characters \
 * 	from b.
 * @param b The string to copy.
 * @param n The number of non-white-space characters to copy from b to a.
 */
char *
stringTrimCopy(char *a, const char *b, int n)
{
        register int i, m;
	register char *pa = a;

        while(*b != '\0' && isspace((int)*b)) b++;

        for(m = (int)strlen(b)-1; m >= 0 && isspace((int)b[m]); m--);
        m++;

	n--;
        for(i = 0; i < n && i < m; i++) *a++ = *b++;
        *a = '\0';
	return pa;
}

/**
 * Trim and convert a string to a quark (int).
 * @param a The character string.
 * @return The quark value.
 */
int
stringTrimToQuark(const char *a)
{
	int m, quark;

        while(*a != '\0' && isspace((int)*a)) a++;

        for(m = (int)strlen(a)-1; m >= 0 && isspace((int)a[m]); m--);
        m++;
	quark = stringNToQuark(a, m);
	return quark;
}

/**
 * Seach program arguments for a parameter string value. Returns the last
 * occurrence of the string value for name=value.
 * @param argc The number of strings in argv.
 * @param argv An array of strings to search for name=value.
 * @param name The parameter name.
 * @return The string value or NULL if name is not found. Free the returned \
 * pointer after use.
 */
char *
stringGetArgv(int argc, const char **argv, const char *name)
{
	int i, j;
	char *s, *c;
	const char *a;

	/* start at end
	*/
	for(i = argc-1; i > 0; i--) {
	    a = argv[i];
	    for(j = 0; a[j] != '\0' && a[j] != '='; j++);
	    if(a[j] == '=') {
		s = (char *)malloc(strlen(argv[i]) + 3);
		strncpy(s, a, (size_t)(j+1));
		s[j+1] = '"';
		s[j+2] = '\0';
		strcat(s, a+j+1);
		strcat(s, "\"");
		if((c = stringGetArg(s, name)) != NULL) {
		    free(s);
		    return c;
		}
		free(s);
	    }
	}
	return NULL;
}

/**
 * Seach program arguments for a parameter int value. Returns the last
 * occurrence of the int value for name=value.
 * @param argc The number of strings in argv.
 * @param argv An array of strings to search for name=value.
 * @param name The parameter name.
 * @param value The int value found.
 * @return 1 if name was found and the value could be converted to a double. \
 *	Returns 0 otherwise.
 * @see stringToDouble
 */
int
stringGetDoubleArgv(int argc, const char **argv, const char *name,double *value)
{
	char *arg;

	if((arg = stringGetArgv(argc, argv, name)) != NULL) {
	    int err = stringToDouble(arg, value);
	    free(arg);
	    return err;
	}
	return False;
}

/**
 * Seach program arguments for a parameter long value. Returns the last
 * occurrence of the long value for name=value.
 * @param argc The number of strings in argv.
 * @param argv An array of strings to search for name=value.
 * @param name The parameter name.
 * @param value The long value found.
 * @return 1 if name was found and the value could be converted to a long. \
 *	Returns 0 otherwise.
 * @see stringToLong
 */
int
stringGetLongArgv(int argc, const char **argv, const char *name, long *value)
{
	char *arg;

	if((arg = stringGetArgv(argc, argv, name)) != NULL) {
	    int err = stringToLong(arg, value);
	    free(arg);
	    return err;
	}
	return False;
}

/**
 * Seach program arguments for a parameter int value. Returns the last
 * occurrence of the int value for name=value.
 * @param argc The number of strings in argv.
 * @param argv An array of strings to search for name=value.
 * @param name The parameter name.
 * @param value The int value found.
 * @return 1 if name was found and the value could be converted to a int. \
 *	Returns 0 otherwise.
 * @see stringToInt
 */
int
stringGetIntArgv(int argc, const char **argv, const char *name, int *value)
{
	char *arg;

	if((arg = stringGetArgv(argc, argv, name)) != NULL) {
	    int err = stringToInt(arg, value);
	    free(arg);
	    return err;
	}
	return False;
}

/**
 * Seach program arguments for a parameter boolean value. Returns the last
 * occurrence of the boolean value for name=value.
 * @param argc The number of strings in argv.
 * @param argv An array of strings to search for name=value.
 * @param name The parameter name.
 * @param value The boolean value found.
 * @return 1 if name was found and the value could be converted to a boolean. \
 *	Returns 0 otherwise.
 * @see stringToBool
 */
int
stringGetBoolArgv(int argc, const char **argv, const char *name, int *value)
{
	char *arg;

	if((arg = stringGetArgv(argc, argv, name)) != NULL)
	{
	    int err = stringToBool(arg, value);
	    free(arg);
	    return err;
	}
	return False;
}

/**
 * Read the next line. Read up to n characters of the next line of input.
 * Characters beyond n are ignored and will be skipped by a subsequent call.
 * @param fp The FILE pointer of an open file.
 * @param line A character string of length >= n+1 to receive the line.
 * @param n The maximum number of characters to read.
 * @return 0 for success, EOF for end-of-file.
 */
int
stringGetLine(FILE *fp, char *line, int n)
{
	int c = '\0', nc = 0;

	while(n > 0 && (c = getc(fp)) != EOF && c != '\n')
	{
	    n--;
	    *line++ = c;
	    nc++;
	}
	*line = '\0';

	if(c == '\n')
	{
	    return(0);
	}
	else if(c == EOF)
	{
	    return( nc > 0 ? 0 : EOF);
	}

	while((c=getc(fp)) != '\n' && c != EOF);
	return(0);
}

/**
 * Return the string suffix as a quark. The suffix is all characters after the
 * last period '.'. Return a quark representation of a zero length string (""),
 * if the input string does not contain a period.
 * @param filename The input string.
 * @return A quark representation of the suffix.
 */
int
stringGetSuffix(const char *filename)
{
	int i;
	int n = strlen(filename);

	for(i = n-1; i >= 0; i--) {
	    if(filename[i] == '.') {
		return stringToQuark(filename+i+1);
	    }
	}
	return stringToQuark("");
}

/**
 * Return the string suffix. The suffix is all characters after the
 * last period '.'. Return NULL if the input string does not contain a period.
 * @param filename The input string.
 * @return Pointer to the suffix or NULL.
 */
const char *
stringSuffix(const char *filename)
{
	int i;
	int n = (int)strlen(filename);

	for(i = n-1; i >= 0; i--) {
	    if(filename[i] == '.') {
		return filename+i+1;
	    }
	}
	return NULL;
}

/**
 * Return the string prefix as a quark. The prefix is all characters before the
 * last period '.'. Return a quark representation of the entire input string,
 * if it does not contain a period.
 * @param filename The input string.
 * @return A quark representation of the prefix.
 */
int
stringGetPrefix(const char *filename)
{
	int i, j, q;
	int n = (int)strlen(filename);

	for(i = n-1; i >= 0; i--) {
	    if(filename[i] == '.') {
		for(j = i-1; j >= 0 && filename[j] != '/'; j--);
		q = stringNToQuark(filename+j+1, i-j-1);
		return q;
	    }
	}
	return stringToQuark(filename);
}

/**
 * Return the string directory as a quark. The directory is all characters
 * before the last '/'.  Return a quark representation of a zero length string
 * (""), if the input string does not contain a '/'.
 * @param filename The input string.
 * @return A quark representation of the prefix.
 */
int
stringGetDir(const char *filename)
{
	int i, q;
	int n = (int)strlen(filename);

	for(i = n-1; i >= 0; i--) {
	    if(filename[i] == '/') {
		q = stringNToQuark(filename, i);
		return q;
	    }
	}
	return stringToQuark("");
}

/**
 * Check for a string ending.
 * @param s The input string.
 * @param ending A substring to compare to the ending of s.
 * @return 1 if the end of s matches the ending. Returns 0 if it does not match.
 */
int
stringEndsWith(const char *s, const char *ending)
{
	int n = (int)strlen(s);
	int m = (int)strlen(ending);

	if(m > n) return 0;

	return (!strcmp(s+n-m, ending)) ? 1 : 0;
}

/**
 * Check for a string ending (case insensitive).
 * @param s The input string.
 * @param ending A substring to compare to the ending of s.
 * @return 1 if the end of s matches the ending. Returns 0 if it does not match.
 */
int
stringCaseEndsWith(const char *s, const char *ending)
{
	int n = (int)strlen(s);
	int m = (int)strlen(ending);

	if(m > n) return 0;

	return (!strcasecmp(s+n-m, ending)) ? 1 : 0;
}

char *
stringcpy(char *dest, const char *src, int n)
{
	if(dest != src) {
	    memset((void *)dest, 0, n);
	    strncpy(dest, src, n);
	}
	if(n > 0) dest[n-1] = '\0';
	return dest;
}

int
stringCaseMatch(const char *a, const char *b)
{
	char *s1=NULL, *s2=NULL;
	int ret;

	if(a) {
	    s1 = stringCopy(a);
	    stringToUpper(s1);
	}
	else {
	    s1 = NULL;
	}
	if(b) {
	    s2 = stringCopy(b);
	    stringToUpper(s2);
	}
	else {
	    s2 = NULL;
	}
	ret = stringMatch(s1, s2);
	if(s1) free(s1);
	if(s2) free(s2);
	return ret;
}

int
stringMatch(const char *a, const char *b)
{
	int n;
	if(!a) return False;
	if(a[0] == '\0'  || !strcmp(a, "*")) return True;
	if(!b || b[0] == '\0' || !strcmp(b, "*")) return True;

	n = strlen(b);

	if(b[0] == '*')
	{
	    if(b[n-1] == '*') {
		char *c;
		if(n <= 2) return True;
		c = stringNCopy(b+1, n-2);
		if(!strstr(a, c)) {
		    free(c);
		    return False;
		}
		free(c);
	    }
	    else {
		if(!stringEndsWith(a, b+1)) return False;
	    }
	}
	else if(b[n-1] == '*') {
	    if(strncmp(a, b, n-1)) return False;
	}
	else {
	    if(strcmp(a, b)) return False;
	}

	return True;
}

/**
 * @short Extracts the file name (characters after the last '/')
 * from 'pathName'. This function is equivalent to the Unix/Linux
 * basename utility.
 *
 * IMPORTANT - 'baseName' must be freed by the calling function.
 *
 * @param pathName  The name of the file.
 */
char *
stringBaseName (const char * pathName)
{
  char   *ptr    = NULL;
  char   *result = NULL;
  size_t  b_len  = 0;

  /* check precondition */
  if (pathName == NULL)
    {
      logErrorMsg(LOG_ERR, "stringBaseName: precondition failed.");
      return NULL;
    }

  /* find the last occurrence of the '/' (path delimiter) in
   * 'pathName' */
  ptr = (char *) strrchr ((const char *) pathName, (int) '/');
  if (ptr == NULL)
    {
      /* the 'pathName' doesn't contain the path */
      b_len = strlen (pathName);
      ptr   = (char *) pathName;
    }
  else
    {
      /* remove the path delimiter, i.e., '/' */
      b_len = strlen (++ptr);
    }

  /* allocate memory for the 'staName' */
  if ((result = (char *) malloc (b_len + 1)) == (void *) 0)
    {
      logErrorMsg(LOG_ERR, "stringBaseName: malloc failed.");
      return NULL;
    }
  strncpy (result, ptr, b_len);
  result [b_len] = '\0';
  return result;
} /* stringBaseName */

int
stringUpperToQuark(const char *s)
{
    char *c = stringCopy(s);
    int q = stringToQuark(stringToUpper(c));
    if(c) free(c);
    return q;
}

int
stringArg(const char *s, const char *name, char **value)
{
    int n = (int)strlen(name);

    while(*s != '\0' && isspace((int)*s)) s++;

    if( !strncasecmp(s, name, n) )
    {
	/* the character after the name must be white space, '.', or '='. */
	char *c = (char *)(s + n);
	if(!isspace((int)*c) && *c != '.' && *c != '=') return 0;
	if(*c == '.') {
	    c++;
	    while(*c != '\0' && isspace((int)*c)) c++;
	}
	else {
	    while(*c != '\0' && isspace((int)*c)) c++;

	    if(*c == '=') {
		c++;
		while(*c != '\0' && isspace((int)*c)) c++;
	    }
	}
	*value = c;
	return 1;
    }
    return 0;
}
