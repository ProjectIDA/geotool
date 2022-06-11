/*
 * AUTHOR
 *      J. Coyne
 *      SAIC
 */
#include "config.h"
#include <string.h>
#include <ctype.h> 

#include "libstring.h"

#ifdef TEST
main()
{
	char	samp1[90], samp2[90], samp3[90];
	char	samp1a[90], samp2a[90], samp3a[90];

	strcpy(samp1, "FIA0 GEC2");

	check_char(samp1, samp1a, "sta");
	printf("in = %s\nout = %s\n", samp1, samp1a);

	strcpy(samp2, "GEC%,");

	check_char(samp2, samp2a, "sta");
	printf("in = %s\nout = %s\n", samp2, samp2a);

	strcpy(samp3, "sta = FIA2");

	check_char(samp3, samp3a, "sta");
	printf("in = %s\nout = %s\n", samp3, samp3a);
}
#endif

/**
 * Create the where clause of a query from a list of stations or channels.
 */
int
check_query_char(const char *clause, char *new_clause, int size,
		const char *type, int lower_case)
{
	char	_like[8];
	int	i, k, n, n2, len, len2;
	int	need_like, have_like, have_refsta;

       	strncpy(_like, "", 8);
        len = len2 = i = k = n = n2 = 0;

	if (clause == NULL) 
	{
		strncpy(new_clause, "", size);
		return(-1);
	}

	len = strlen(clause);
	if (len < 1)
	{
		strncpy(new_clause, "", size);
		return(-1);
	}
	/* maybe check for "and", and if found, return ? */

	need_like = have_like = 0;

	/* skip over whitespace */
	n = 0;
	n2 = 0;
	while(clause[n] != '\0' && (clause[n] == ' ' || clause[n] == '\t')) n++;

	strncpy(new_clause, type, size);

	len2 = strlen(new_clause);
	for (k=0, i=n; i< n + len2; i++, k++)
	{
		if (tolower(new_clause[k]) != tolower(clause[i]))
		{
			break;
		}
	}

	if (i == (n + len2))
	{
		n += len2;
	}

	n2 = len2;

	have_refsta = 0;
	if (!strcmp(type, "sta"))
	{
		if (tolower(clause[n])   == 'r' &&
		    tolower(clause[n+1]) == 'e' &&
		    tolower(clause[n+2]) == 'f' &&
		    tolower(clause[n+3]) == 's' &&
		    tolower(clause[n+4]) == 't' &&
		    tolower(clause[n+5]) == 'a')
		{
			have_refsta = 1;
/*
			strcpy(new_clause, "sta in (select sta from idcdev.site where refsta ");
*/
			strncpy(new_clause,
		"sta in (select sta from idcdev.affiliation where net ", size);
			n2 = 53;
			n += 6;
		}
	}

	
	while(clause[n] != '\0' && (clause[n] == ' ' || clause[n] == '\t')) n++;

	for (i=n; i<len; i++)
	{	
		if (clause[i] == '*' || clause[i] == '%')
		{
			need_like = 1;
		}
	}

	if (clause[n] == '=')
	{
		new_clause[n2++] = ' ';
		new_clause[n2++] = '=';
		new_clause[n2++] = ' ';
		n++;
	}
	else 
	{
		strncpy(_like, "like ", 8);
		for (i=n, k=0; i < n + 5; i++, k++)
		{
			if (tolower(clause[i]) != _like[k])
				break;
		}
		if (i == (n+5))
		{
			n += 5;
			have_like = 1;
		}
		if (have_like || need_like)
		{
			new_clause[n2++] = ' ';
			new_clause[n2++] = 'l';
			new_clause[n2++] = 'i';
			new_clause[n2++] = 'k';
			new_clause[n2++] = 'e';
			new_clause[n2++] = ' ';
		}
		else
		{
			strncpy(_like, "in ", 8);
			for (i=n, k=0; i < n + 3; i++, k++)
			{
				if (tolower(clause[i]) != _like[k])
					break;
			}
			if (i == (n+3))
			{
				n += 3;
			}
			new_clause[n2++] = ' ';
			new_clause[n2++] = 'i';
			new_clause[n2++] = 'n';
			new_clause[n2++] = ' ';
		}
	}

	while(clause[n] != '\0' &&
	    (clause[n] == ' ' || clause[n] == '\t' || clause[n] == '\'')) n++;
	

	k = 0;
	new_clause[n2++] = '(';

	while (clause[n] != '\0' &&
		(clause[n] == ' ' || clause[n] == '\t' || 
		 clause[n] == ',' || clause[n] == '\'' ||
		 clause[n] == '(' || clause[n] == ')')) n++;

	while (n < len)
	{
		if (clause[n] == '\'') n++;
		if(n == len) break;
		if (k > 0)
		{
			new_clause[n2++] = ',';
			new_clause[n2++] = '\'';
		}
		else
		{
			new_clause[n2++] = '\'';
		}

		while((!isalnum((int)clause[n]) && clause[n] != '%' &&
			clause[n] != '*') && n < len) n++;

		while ((isalnum((int)clause[n]) || clause[n] == '%' ||
			clause[n] == '*') && n < len)
		{
			if (lower_case == 1)
				new_clause[n2++] = tolower(clause[n++]);
			else if(lower_case == 0)
				new_clause[n2++] = toupper(clause[n++]);
			else 
				new_clause[n2++] = clause[n++];
		}
		new_clause[n2] = '\'';
		n2++;
		k++;

		while (clause[n] != '\0' && 
			(clause[n] == ' ' || clause[n] == '\t' || 
			 clause[n] == ',' || clause[n] == '\'' ||
			 clause[n] == '(' || clause[n] == ')')) n++;
	}
	new_clause[n2++] = ')';
	if (have_refsta) 
		new_clause[n2++] = ')';
	new_clause[n2] = '\0';
	return(1);
}

/**
 * Create the where clause of a query from a list of double values.
 */
int
check_query_int(const char *clause, char *new_clause, int size,
		const char *type)
{
	int	i, k, n, n2, len, len2;
	char	_between[16];
	int	have_between;

	if (clause == NULL) 
	{
		strncpy(new_clause, "", size);
		return(-1);
	}

	len = strlen(clause);
	if (len < 1)
	{
		strncpy(new_clause, "", size);
		return(-1);
	}
	/* maybe check for "and", and if found, return ? */


	/* skip over whitespace */
	n = 0;
	n2 = 0;
	while(clause[n] != '\0' && (clause[n] == ' ' || clause[n] == '\t')) n++;

	len2 = strlen(type);
	for (k=0, i=n; i< n + len2; i++, k++)
	{
		if (tolower(type[k]) != tolower(clause[i]))
		{
			break;
		}
	}

	if (i == (n + len2))
	{
		n += len2;
	}
	strncpy(new_clause, type, size);
	n2 = len2;
	
	while(clause[n] != '\0' && (clause[n] == ' ' || clause[n] == '\t')) n++;

	have_between = 0;
	if (clause[n] == '=')
	{
		new_clause[n2++] = ' ';
		new_clause[n2++] = '=';
		new_clause[n2++] = ' ';
		n++;
	}
	else 
	{
		strncpy(_between, "between", 16);
		for (i=n, k=0; i < n + 7; i++, k++)
		{
			if (tolower(clause[i]) != _between[k])
				break;
		}
		if (i == (n+7))
		{
			n += 7;
			have_between = 1;
		}
		if (have_between)
		{
			new_clause[n2++] = ' ';
			new_clause[n2++] = 'b';
			new_clause[n2++] = 'e';
			new_clause[n2++] = 't';
			new_clause[n2++] = 'w';
			new_clause[n2++] = 'e';
			new_clause[n2++] = 'e';
			new_clause[n2++] = 'n';
			new_clause[n2++] = ' ';
		}
		else
		{
			strncpy(_between, "in", 16);
			for (i=n, k=0; i < n + 2; i++, k++)
			{
				if (tolower(clause[i]) != _between[k])
					break;
			}
			if (i == (n+2))
			{
				n += 2;
			}
			new_clause[n2++] = ' ';
			new_clause[n2++] = 'i';
			new_clause[n2++] = 'n';
			new_clause[n2++] = ' ';
		}
	}

	while (clause[n] != '\0' &&
	    (clause[n] == ' ' || clause[n] == '\t' || clause[n] == '\'')) n++;
	

	k = 0;
	if (!have_between) 
	{
		new_clause[n2++] = '(';
	}
	while (n < len)
	{
		if (k > 0)
		{
			new_clause[n2] = ',';
			n2++;
		}

		while ((isalnum((int)clause[n]) || clause[n] == '%') && n < len)
		{
			new_clause[n2++] = clause[n++];
		}
		k++;

		while(clause[n] != '\0' &&
			(clause[n] == ' ' || clause[n] == '\t' || 
			 clause[n] == ',' || clause[n] == '\'')) n++;
	}
	if (!have_between) new_clause[n2++] = ')';
	new_clause[n2] = '\0';
	return(1);
}
