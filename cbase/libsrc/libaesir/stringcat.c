/*
 * NAME
 *      stringcat.c
 * 
 * FILE 
 *      stringcat.c
 *
 * SYNOPSIS
 *      char *stringcat(char *old, char *add);
 *
 *      char *sql_char_list(char *string);
 *
 *      char *xstrtok(char *str, char *delims, char *pair_delims);
 *      
 *      int   vstring(char *string, char **strarray, char *delims);
 *
 * DESCRIPTION
 *
 *      stringcat() appends the string pointed to by <add> to the string
 *      referenced by <old> and returns a pointer to a
 *      the concatenated string.  The memory referenced by <old> is 
 *      realloc()'ed.
 *
 *      sql_char_list() takes a string of space-delimited character elements 
 *      and coverts them to a parenthesis-enclosed string of single-quoted
 *      elements suitable for parsing by SQL. E.g the string "this is a test" 
 *      is converted to "('this', 'is', 'a', 'test')";
 *      "'This is ' a 'te'st" is converted to
 *      "('This is ', 'a', 'te'st')"
 *
 *      xstrtok() extends the token processing provided by strtok(3) to 
 *      allow suppression within specified delimiters.  For consecutive
 *      calls to xstrtok, token extraction is suppressed for the substring
 *      within the <pair> delimiters.
 *
 *      vstring() returns an array of dynamically allocated tokens parsed
 *      according to input delimiters; single- and double-quoted substrings
 *      are interpreted as a single token.  The number of tokens in the 
 *      array are returned, the array is, additionally NULL terminated.
 *      leading single/double quotes are stripped along with their matching
 *      trailing single/double quotes.  Embedded single/double quotes are 
 *      left in place.  E.g.:
 *      "This is a test" -> 4 strings: "This" "is" "a" "test".
 *      "'This is" a ' test'" -> 3 strings: "This is" "a" " test".
 *      "Thi's is a te'st" -> 1 string "Thi's is a te'st".
 *
 * DIAGNOSTICS
 *
 *      stringcat(), sql_char_list() return a NULL pointers if 
 *      malloc() or realloc() fail.
 *      
 *      vstring() return -1 if malloc() or realloc() fail.
 *
 * FILES
 *
 * NOTES
 *
 *      vstring() is not written to efficiently deal with large numbers of
 *      strings.  Alternating malloc() and realloc() will fragment the
 *      heap.
 *
 * SEE ALSO
 *      add_to_str() fom an alternative to stringcat().
 *
 * AUTHOR
 *	Jeff Given Jan 93
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>

#include "aesir.h"
#include "libaesir.h"

char *
stringcat(p, s)
char *p;
char *s;
{
	if(s == (char *) NULL)
	{
		return(p);
	}
	if(! strlen(s))
	{
		return(p);
	}

	if(p == (char *) NULL)
	{
		p = STRALLOC(s);
	}
	
	else
	{
		p = UREALLOC(p, char, strlen(p) + strlen(s) + 2);
		
		strcat(p, s);
	}
	
	return(p);
	
}

char *
sql_char_list(clist)
char *clist;
{
	int i;
	char        *ret;
	char       **cpp;

	if(clist == (char *) NULL)
	{
		return((char *) NULL);
	}

	if(vstring(clist, &cpp, " ,\t\n") <= 0)
	{
		return((char *) NULL);
	}
	
	for(i=0, ret = stringcat((char *) NULL, "('");
	    cpp[i] != (char *) NULL;
	    i++)
	{
		if(! (ret = stringcat(ret, cpp[i]))) break;
		if(! (ret = stringcat(ret, "', '"))) break;
	}
	
	if(ret) ret[strlen(ret)-3] = '\0';
		
	if(ret)  ret = stringcat(ret, ")");

	for(i=0; cpp[i] != (char *) NULL; i++)
	{
		UFREE(cpp[i]);
	}
	
	UFREE(cpp);

	return(ret);
}

char *
xstrtok(t, delims, pair)
char *t;
char *delims;
char *pair;
{
	int i;
	
	int          l;
	static char *s;
	char        *p;
	char        *q = NULL;
	char        *to_new;
	char         mark;

	if(strlen(delims) == 0)         /*  No token delimiters */
	{
		return(t);
	}

	if(pair != (char *) NULL)
	{
		if(strlen(pair) !=0 && strpbrk(pair, delims) != (char *) NULL)
		{
	       /*
		* token delimiters and pair delimiters intersect
		* this is actually an error
		*/
			s = (char *) NULL;
			return(char *) NULL;
		}
	}

	if( t != (char *) NULL)
	{
		l = strspn(t, delims);
		for(i = 0; i < l; t++, i++);
		s = t;
	}

	if(s == (char *) NULL)
	{
		return((char *) NULL);
	}
	
	if(! strlen(s))
	{
		s = (char *) NULL;
		return((char *) NULL);
	}

	p = strpbrk(s, delims);

	if(pair != (char *) NULL)
	{
		if(strlen(pair) != 0) 
		{
			q = s;
			while((q = strpbrk(q, pair)) != (char *) NULL)
			{
				if(q > s && q[-1] != '\\') break;
				else if(q == s)            break;
				q++;
			}
		}
		else    q = (char *) NULL;
	}

	if( p == (char *) NULL)
	{
		to_new = s;
		s = (char *) NULL;
		return(to_new);
	}

	while(p != (char *) NULL && q != (char *) NULL && q < p)
	{
		mark = *q++;
		
		while(*q != '\0')
		{
			if(*q == mark && q[-1] != '\\') break;
			q++;
		}

		q++;

		if(p < q)
		{
			p = strpbrk(q, delims);
			if(p == (char *) NULL) break;
		}

		while((q = strpbrk(q, pair)) != (char *) NULL)
		{
			if(q > s && q[-1] != '\\') break;
			else if(q == s)            break;
			q++;
		}
	}

	if( p == (char *) NULL)
	{
		to_new = s;
		s = (char *) NULL;
		return(to_new);
	}
	else
	{	
		l = strspn(p, delims);
		for(i = 0; i < l; *p = '\0', p++, i++);
		to_new = s;
		s = p;
		return(to_new);
	}
}

int
vstring(old, values, delims)
char *old;
char ***values;
char *delims;
{
	char **a;
	int i, j, inc, size;
	char *p, *t;

	if(old == (char *) NULL)
	{
		*values = (char **) 0;
		return(-1);
	}

	t = STRALLOC(old);

	inc  = 20;
	size = inc;

	a = UALLOC(char *, size + 1);
	
	for (p = xstrtok(t, delims, "\"\'"), i=0;
	     p != (char *) NULL;
	     p = xstrtok((char *)0, delims, "\"\'"), i++)
	{
		/*
		 *  strip out pair delimiters if present
		 */

		if(strpbrk(p, "\"\'") == p)
		{
			j = strlen(p) - 1;
			if(p[j] == p[0]) p[j] = '\0';
			++p;
		}

		if(i == size)
		{
			size += inc;
			a = (char **) UREALLOC(a, char *, size + 1);
		}
		a[i] = STRALLOC(p);
	}
	
	a[i] = (char *) NULL;

	*values = a;

	UFREE(t);

	return(i);
}
