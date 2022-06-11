/* 
 * NAME
 *	setcvar
 *
 * SYNOPSIS
 *
 *	int
 *	setcvar(ac,av)		
 *	int ac;		(i) Argument count
 *	char **av;	(i) Argument vector
 *
 *      int
 *      getcvar(name, type, val)
 *      char *name;     (i) variable name
 *      char *type;     (i) variable type ["s", "d", "f", "F"]
 *      void *value;    (o) variable value
 *
 *      void 
 *      addcvar(n, lines)
 *      int    n;       (i) number of variable lines to be parsed
 *      char **lines;   (i) variable lines
 *      
 * DESCRIPTION
 *
 *	Parse command line arguments and argument strings into the cvar table
 *      Retrieve and convert values from the cvar table;
 *
 * NOTES
 *
 *	The cvar table MUST BE INITIALIZED by var_init() FIRST
 *	
 * SEE ALSO
 *
 *	libtable.3 (var_init(), var_get(), var_set() )
 *	libpar.3
 *
 * AUTHOR
 *	
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>	/* For atof(), atoi() prototypes */
#include "cdefs.h"
#include "table.h"	/* For setcvar() prototype */

#define MAXLINE		1024	/* max length of line in par file */
#define MAXNAME		64	/* max length of name */
#define MAXVALUE	8192	/* max length of value */
#define MAXFILENAME	64	/* max length of par file name */
#define MAXVECTOR	10	/* max # of elements for unspecified vectors */
#define MAXPARLEVEL	8	/* max recursion level for par files */

#define INIT	 1	/* bits for FLAGS (ext_par1.argflags) */
#define STOP	 2
#define LIST	 4
#define END_PAR	 8
#define VERBOSE	16

#define SETCVAR_ERROR 100

struct ext_par1		/* global variables for getpar */
{
	char *progname;
	int argflags;
}	ext_par1;


/* abbreviations: */

#define PROGNAME	ext_par1.progname
#define FLAGS		ext_par1.argflags

/* ... to support partable stuff */

#define MAXCOLUMNS  128         /* max column in a table at once */
#define MAXTOKENS   128         /* should be same as max columns */

static char cv_columns[MAXCOLUMNS][MAXNAME]; /* these are the current cols */
static int  cv_ncolumns;                     /* how many columns */
static char cv_delimit[MAXNAME] = " \t|";     /* column delimiters */
static char cv_whitespace[MAXNAME] = " \t\n"; /* what's white space */
static char cv_quote[MAXNAME] = "\"\'";       /* what's a quote */
static int  cv_lineno_c = '@';                /* row number parameter */
static char cv_tbl_index_fmt[MAXNAME]  = "[%s]."; /* table/column naming 
						     format for named tables */
static char cv_col_index_fmt[MAXNAME] = "[%s]";   /* column naming format 
						     for unnamed tables */

#define CV_START(a, b) (((a) - (b) > 0) ? 0 : 1)


/* Local prototypes */
Proto( static void,	cv_add_entry, ( char *name, char *value ));
Proto( static void,	cv_do_environment, ( int ac, char **av ));
Proto( static int,	cv_do_par_file, ( char *fname, int level ));
Proto( static void,	cv_setcvar_err, ( char *subname, char* mess,
					 char *a1 ));
Proto( static void,	cv_subpar, ( char **apl, char **apv, int start ));
Proto( static char,	*cv_fgets, ( char *line, int maxline, FILE *fp ));

Proto (static void, cv_do_par_line, (char *line, int level));

/* new local partable protos */
Proto (static char *, cv_xstrtok, (char *t, char *delims, char *pair));
Proto (static int, cv_vstring, (char  *old, char **values, int    maxvalues));
Proto (static void, cv_parcpy, (char  *pv, char **apl));
Proto (static void, cv_do_columns, (char *line));
Proto (static int, cv_do_rows,(char *line, char *fmt, int   num));
Proto (static void, cv_do_par_table, (FILE  *fp, char  *cmd, int level));

/* global environment */
extern char **environ;

int
setcvar(ac,av)
int ac;
char **av;
   {
	register char *pl, *pn, *pv;
	char  t, name[MAXNAME], value[MAXVALUE];

	int addflags, endsetpar = 0;
	
	char  *apl, *apv;

	PROGNAME= *av;
	FLAGS= INIT;

	/*
	 * The environment is parsed unless NOENV is the first argument.
	 */
	if( (ac > 1) && ( strcmp(av[1], "NOENV") == 0 ))
		;
	else
		cv_do_environment(ac,av);


	while(--ac > 0 && endsetpar == 0)
	{
		av++;
		pl= *av;
		while(*pl == ' ' || *pl == '\t') pl++;
		/* get name */
		pn= name;
		while(*pl != '=' && *pl != '\0') *pn++ = *pl++;
		*pn++ = '\0';
		/* get value */
		if(*pl == '=') pl++;
		*value = '\0';
		pv=value;

		if(*pl == '"' || *pl == '\'')
		{
			t= *pl++;
			while(*pl != '\0')
			{
				if(*pl == t)
				{
					if(pl[-1] != '\\') break;
					pv[-1]= t;
					pl++;
				}
				else
				{
					if(*pl == '$')
					{
						apl = pl;
						apv = pv;
						cv_subpar(&apl, &apv, 0);
						pl = apl;
						pv = apv;
					}
					else *pv++ = *pl++;
				}
			}
		}
		else	
		{
			while(*pl) 
				if(*pl == '$')
				{
					apl = pl;
					apv = pv;
					cv_subpar(&apl, &apv, 0);
					pl = apl;
					pv = apv;
				}
				else *pv++ = *pl++;
		}
		*pv= '\0';
		if(name[0] == '-') cv_add_entry("SWITCH",&name[1]);
		else		cv_add_entry(name,value);
		if(strcmp("par",name)==0) /* par file */
			if( cv_do_par_file(value,1) < 0 ) return ac;

	/* Added by Glenn Nelson (nelson@ollie.UCSC.EDU) to allow mixture
	   of getpar() and ordinary command line stuff. */
		if (strcmp("ENDPAR", name) == 0) endsetpar = 1;
	}

	addflags= 0;
	*value= '\0';
	if(var_get("STOP")) addflags |= STOP;
	*value= '\0';
	if(var_get("VERBOSE")) addflags |= VERBOSE;
	*value= '\0';

	if(var_get("LIST"))
	{
		addflags |= LIST;
	}
	*value= '\0';

	FLAGS |= addflags;

	/* Added by Glenn Nelson (nelson@ollie.UCSC.EDU) to allow setpar()
	   to terminate before all command line args are exhausted. */
	return ac;
}


int
getcvar (name,type,val)
char   *name, *type;
void     *val;
{
	register char *sptr;
	double *dbl;
	float *flt;
	int	*i;
	int found;
	char *str;

	/* The following line corrects a common input error */
	if(type[1]=='v') { type[1]= type[0]; type[0]='v'; }

	found=0;

	if((str = (char *) var_get(name)) == (char *) NULL)
	{
		return(0);
	}

	switch(*type)
	{
	case 'd':
		i = (int *) val;
		*i = atoi(str);
		found=1;
		break;
	case 'f':
		flt= (float *) val;
		*flt= atof(str);
		found=1;
		break;
	case 'F':
		dbl= (double *) val;
		*dbl= atof(str);
		found=1;
		break;
	case 's':
		sptr= (char *) val;
		while(*str) *sptr++ = *str++;
		*sptr= '\0';
		found=1;
		break;
	default:
		cv_setcvar_err("getcvar", "unknown conversion type %s",type);
		break;
	}

	return(found);
}

void
addcvar(count, lines)
int   count;
char **lines;
{
    int i;
    
    if(lines)
    {
	for(i=0; i < count; i++)
	{
	    if(! lines[i]) continue;
	    cv_do_par_line(lines[i], 0);
	}
    }
}

static void
cv_add_entry(name,value)	
register char *name, *value;
{
	var_set(name, value);

	return;
}

static void
cv_do_environment(ac,av)
int ac; char **av;
{
	char **ae;
	register char *pl, *pn, *pv;
	char name[MAXNAME], value[MAXVALUE], t;

	ae = environ;

	while(*ae != NULL)
	{
		pl= *ae;
		while(*pl == ' ' || *pl == '\t') pl++;
		/* get name */
		pn= name;
		while(*pl != '=' && *pl != '\0') *pn++ = *pl++;
		*pn = '\0';

		/* get value */
		if(*pl == '=') pl++;
		pv= value;
		if(*pl == '"' || *pl == '\'')
		   {
			t= *pl++;
			while(*pl != '\0')
			   {
				if(*pl == t)
				   {
					if(pl[-1] != '\\') break;
					pv[-1]= t;
					pl++;
				   }
				 else	*pv++ = *pl++;
			   }
		   }
		 else	while(*pl) *pv++ = *pl++;
		*pv= '\0';
		cv_add_entry(name,value);
		ae++;
	}
}

static void
cv_do_par_line(line, level)
char *line;
int   level;
{
	register char *pl, *pn, *pv;
	char t,  name[MAXNAME], value[MAXVALUE];
	char *apl, *apv;
	
	if(! (pl = line)) return;
	
 loop:	while(*pl==' ' || *pl=='\t') pl++;

	/* string over ... return */

	if(*pl=='\0'|| *pl=='\n') return;
	
	/* comments on rest of line */

	if(*pl=='#') return;

	/* get name */
	
	pn= name;
	while(*pl != '=' && *pl != '\0' && *pl != ' '
	      && *pl != '\n'		/* FIX by Glenn Nelson */
	      && *pl != '\t') *pn++ = *pl++;

	*pn = '\0';

	if(*pl == '=') pl++;

	/* get value */

	*value= '\0';
	pv= value;
	
	if(*pl == '"' || *pl == '\'')
	{
		t= *pl++;
		while(*pl != '\0' && *pl != '\n')
		{
			if(*pl == t)
			{
				if(pl[-1] != '\\') 
				{
					pl++;
					break;
				}
				pv[-1]= t;
				pl++;
			}
			else
			{
				if(*pl == '$')
				{
					apl = pl;
					apv = pv;
					cv_subpar(&apl, &apv, 0);
					pl = apl;
					pv = apv;
				}
				else *pv++ = *pl++;
			}
		}
	}
	else   
	{
		while(*pl != '\0' && *pl != '\n'
		      && *pl != '\t' && *pl != ' ') 
			if(*pl == '$')
			{
				apl = pl;
				apv = pv;
				cv_subpar(&apl, &apv, 0);
				pl = apl;
				pv = apv;
			}
			else *pv++ = *pl++;
	}
	*pv= '\0';
	
	cv_add_entry(name,value);
	if(strcmp("par",name) == 0)
		cv_do_par_file(value,level+1);
	goto loop;
}

static int
cv_do_par_file(fname,level)
char *fname;
int level;
   {
	register char *pl;
	char line[MAXLINE];
	char buffer[MAXNAME];
	FILE *file;
	int retval = 0;
 
	if(level > MAXPARLEVEL)
	{
		sprintf(buffer,"%d",level);
		cv_setcvar_err("setcvar","%s (too many) recursive par files",
			       buffer);
		return(-1);
	}
	
		
	if(*fname == '\0') return(0);
	
	if( (file=fopen(fname,"r"))==NULL)
	{
		cv_setcvar_err("setcvar","cannot open par file %s",fname);
		return(-1);
	}
	

	while( cv_fgets(line,MAXLINE,file) != NULL )
	{
		pl= line;

		if( *pl== '\0') continue;

		if(pl[0] == '#') 
		{
			if(pl[1] == '!')
			{
				if(! strncmp(pl, "#!BeginTable", 12))
				{
					cv_do_par_table(file, line, level);
					continue;
				}
			}
			else
				continue;
			
		}

		/* loop over entries on each line */
		cv_do_par_line(pl, level);

	}

	fclose(file);
	return(retval);
   }

static void
cv_setcvar_err(subname,mess,a1)
char *subname, *mess;
char *a1;
{
	fprintf(stderr,"\n***** ERROR in %s[%s] *****\n\t",
		(PROGNAME == NULL ? "(unknown)" : PROGNAME),subname);
	fprintf(stderr,mess,a1);
	fprintf(stderr,"\n");
	return;
}

/*  This allows parameter substitution */
static void
cv_subpar(apl, apv, start)
char **apl, **apv;
int	start;
{
	register char *pl, *pv;
	char     subname[MAXNAME];
	int      valid = 0;
	char     *bpl, *bpv, *tmpp;
	
	if(! start && (*apl)[-1] == '\\')
	{
		(*apv)[-1] = (*apl)[0];
		(*apl)++;
		return;
	}

	pl = *apl;
	pl++;

	if(*pl == '(' )
	{
		pv=subname;
		pl++;
		while(*pl != ')' && *pl != '\0')
		{
			if(*pl == '$')
			{
				bpl = pl;
				bpv = pv;
				cv_subpar(&bpl, &bpv, 0);
				pl = bpl;
				pv = bpv;
			}
			else *pv++ = *pl++;
		}
		*pv = '\0';
		if(*pl == ')' )
		{
			pl++;
			pv = *apv;
			valid = 1;
			if( (tmpp = var_get(subname)) != NULL )
			{
				strcpy(pv, tmpp);
				pv += strlen(pv);
			}
			*apv=pv;
			*apl=pl;
				
		}
	}
	if(valid)
	{
		return;
	}
	else
	{
		**apv = **apl;
		(*apv)++;
		(*apl)++;
	}
}

static char *
cv_fgets(line, maxline, file)
char *line;
int  maxline;
FILE *file;
{
	register char *p;
	register int i;
	int q;

        p = line;
	*p = '\0';

	if    ( (q = getc(file)) == EOF)     return((char *) NULL);
	else if( (unsigned char) q == '\n') return(line);

	*p = (unsigned char) q;
	p++;

	for (i=1; (q = getc(file)) != EOF && i < maxline;)
	{
		*p = (unsigned char) q;
		if(*p == '\0') return(line);
	        if(*p == '\n')
		{
			if( *(p-1) == '\\') 
			{
				p--;
				i--;
			}
			else break;
		}
		else 
		{
			p++;
			i++;
		}
	}
	*p = '\0';
	
	return line;
}

static
char *
cv_xstrtok(t, delims, pair)
char *t;
char *delims;
char *pair;
{
	register   int i;
	
	int          l;
	static char *s;
	char        *p;
	char        *q = NULL;
	char        *new;
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
		new = s;
		s = (char *) NULL;
		return(new);
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
		new = s;
		s = (char *) NULL;
		return(new);
	}
	else
	{	
		l = strspn(p, delims);
		for(i = 0; i < l; *p = '\0', p++, i++);
		new = s;
		s = p;
		return(new);
	}
}

static
int
cv_vstring(old, values, maxvalues)
char  *old;
char **values;
int    maxvalues;
{
	int i, j;
	char *p;

	if(maxvalues <= 0) return -1;
	
	if(old == (char *) NULL)
	{
		values[0] = (char *) 0;
		return(-1);
	}

	for (p = cv_xstrtok(old, cv_delimit, cv_quote), i=0;
	     p != (char *) NULL && i < maxvalues -1 ;
	     p = cv_xstrtok((char *)0, cv_delimit, cv_quote), i++)
	{
		/*
		 *  strip out pair delimiters if present
		 */

		if(strpbrk(p, cv_quote) == p)
		{
			j = strlen(p) - 1;
			if(p[j] == p[0]) p[j] = '\0';
			++p;
		}

		values[i] = p;
	}
	
	values[i] = (char *) NULL;

	return(i);

}

static void
cv_parcpy(pv, apl)
char  *pv;
char **apl;
{
	char *pl = *apl;
	char t;
	int   start;

	if(*pl == '"' || *pl == '\'')
	{
		t= *pl++;
		while(*pl != '\0' && *pl != '\n')
		{
			if(*pl == t)
			{
				if(pl[-1] != '\\') 
				{
					pl++;
					break;
				}
				pv[-1]= t;
				pl++;
			}
			else
			{
				if(*pl == '$')
				{
					start = CV_START(pl, *apl);
					cv_subpar(&pl, &pv, start);
				}
				else *pv++ = *pl++;
			}
		}
	}
	else
	{
		while(*pl != '\0' && *pl != '\n'
		      && *pl != '\t' && *pl != ' ') 
			if(*pl == '$')
			{
				start = CV_START(pl, *apl);
				cv_subpar(&pl, &pv, start);
			}
			else *pv++ = *pl++;
	}
	*pv= '\0';
	*apl = pl;
	return;
}

static void
cv_do_columns(line)
char *line;
{
	char *pl;
	char *tokens[MAXCOLUMNS + 1];
	int   i, n;

	n = cv_vstring(line, tokens, MAXCOLUMNS);
	 
	if(! n) return;
	
	/*
	 *  we have "n" tokens, some of which are white space and indicate
         *  that the corresponding data field shall be ignored.
	 */

	for(i = 0; i < n; i++)
	{
		pl = cv_xstrtok(tokens[i], cv_whitespace, cv_quote);
		if(pl)
			cv_parcpy(cv_columns[i], &pl);
		else
			cv_columns[i][0] = '\0';
	}
	
	cv_ncolumns = i;

	return;
}

static int
cv_do_rows(line, fmt, num)
char *line;
char *fmt;
int   num;
{
	char *pl, *ps;
	char *tokens[MAXCOLUMNS + 1];
	char value[MAXVALUE];
	char name[MAXNAME];
	int   i, n;
	char  lineno[MAXNAME];

	/*
	 *  set the line number indicator
	 */

	sprintf(lineno, "%d", num);
	
	if((pl = strrchr(line, cv_lineno_c)))
	{
		ps = pl + 1;
		cv_parcpy(value, &ps);
		strncpy(lineno, value, sizeof(lineno));
		lineno[MAXNAME-1] = '\0';
		while(pl < ps) *pl++ = cv_whitespace[0];
	}
	
	/*
	 *  get the first cv_ncolumns tokens from the line.
	 *  the rest are ignored.
	 */

	n = cv_vstring(line, tokens, cv_ncolumns + 1);
	 
	if(! n) return(0);
	
	for(i = 0; i < n; i++)
	{
		/*
		 * use <value> as buffer here
		 */

		if(cv_columns[i][0] == '\0') continue;

		/*
		 *  build the column names
		 *  use successive calls to sprintf with 
		 *  escaped %'s to get the position correct
		 *
		 *  1st to write line number ...
		 */

		sprintf(value, fmt, lineno);

		if(strlen(value) > (size_t) (MAXNAME -1) )
		{
			cv_setcvar_err("setcvar:", "column name in table too \
long: %s", value);
		}

  		strcpy(name, value);
		
		/* 
		 * 2nd to write the column name
		 */

		sprintf(value, name, cv_columns[i]);

		if(strlen(value) > (size_t) (MAXNAME -1) )
		{
			cv_setcvar_err("setcvar:", "column name in table too \
long: %s", value);
		}

		strcpy(name, value);

		/*
		 * got the name, go after the value
		 */

		pl = cv_xstrtok(tokens[i], cv_whitespace, cv_quote);
		
		if(pl)
		{
			cv_parcpy(value, &pl);
		
			cv_add_entry(name, value);
		}
	}

	return(1);
}

static void
cv_do_par_table(fp, cmd, level)
FILE  *fp;
char  *cmd;
int    level;
{
	char *pl, *pn;
	char name[MAXNAME];
	char table_name[MAXNAME];
	char *table_name_p;
	char idx_fmt[MAXNAME];
	char line[MAXLINE];
	FILE  *fptrs[MAXPARLEVEL];
	int   n_fp;
	int   nrows;
	char	*tmpp;
	char buffer[MAXNAME];
	

	/*
	 * first parse the "cmd" syntax
	 *
	 * strip off the "#!BeginTable" and parse as an ordinary
	 * par
	 */

	/*
	 * syntax is #!BeginTable <name> more pars...
	 */

	/*
	 * first we want the table name
	 */

	/*
	 * strip first token, which has the "#!BeginTable" field
	 */

	if ( !(pl = cv_xstrtok(cmd, "\t ", NULL))) return;
	
	/* 
	 * get the name if available 
	 */

	pl = cv_xstrtok(NULL,"\t ", NULL);
	
	/*
	 * now the name can be null, which means the table has no name
	 * if its not null, check it for validity here
	 */

	table_name[0] = '\0';
	table_name_p  = NULL;

	if( pl )
	{
		/*
		 * there is a table name
		 */

		table_name_p = table_name;

		if(*pl == '\n' || strlen(pl) >= (size_t) MAXNAME) return;

		pn = table_name;

		while(*pl != '\0' && *pl != '\n')
		{
			if(*pl == '$')
			{
				cv_subpar(&pl, &pn, 0);
			}
			else
				*pn++ = *pl++;
		}
		*pn = '\0';
	}
	
	/*
	 *  one more check for valid table name or null it out here.
	 */

	if(table_name_p && ! strncmp(table_name, "#!NONAME", 8))
	{
		table_name[0] = '\0';
		table_name_p = NULL;
	}

	/*
	 * get more pars off the line and parse;
	 */

	if((pl = cv_xstrtok(NULL, "\t ", NULL)))
	{
		cv_do_par_line(pl, level);
	}

	/*
	 * there are no columns now
	 */

	cv_ncolumns = 0;

	/*
	 *  get format indicators
	 */

	
	if ((tmpp = var_get("DELIMITER")) != NULL)
	{
		strcpy (cv_delimit, tmpp);
	}
	if ((tmpp = var_get("WHITESPACE")) != NULL)
	{
		strcpy (cv_whitespace, tmpp);
	}
	if ((tmpp = var_get("QUOTE")) != NULL)
	{
		strcpy (cv_quote, tmpp);
	}
		
	if( !  table_name_p)  /* no table name */
	{
		if ((tmpp = var_get ("COL_INDEX_FMT")) != NULL)
		{
			strcpy (cv_col_index_fmt, tmpp);
		}
		sprintf(idx_fmt, "%%%%s%s",     cv_col_index_fmt);
	}
	else                       /* a table name */
	{
		if ((tmpp = var_get ("TBL_INDEX_FMT")) != NULL)
		{
			strcpy (cv_tbl_index_fmt, tmpp);
		}
		sprintf(idx_fmt, "%s%s%%%%s", table_name, cv_tbl_index_fmt);
	}

	/*
	 *  loop on lines
	 */

	n_fp = 0;
	fptrs[n_fp] = fp;
	nrows = 0;
	
 loop:
	while( cv_fgets(line, MAXLINE, fptrs[n_fp]) != NULL)
	{
		if(line[0] == '#')
		{
			/*
			 * #!FILE <table file> points to another file
			 */

			if(! strncmp(line, "#!File", 6))
			{
				pl = cv_xstrtok(line, "\t ", NULL);
				if(! (pl = cv_xstrtok(NULL, "\t ", NULL)))
				{
					break;
				}
				
				pn = name;
				while(*pl != '\0' && *pl != '\n')
				{
					if(*pl == '$')
					{
						cv_subpar(&pl, &pn, 0);
					}
					else
						*pn++ = *pl++;
				}
				*pn = '\0';

				if(++level > MAXPARLEVEL)
				{
					sprintf(buffer,"%d",level);
					cv_setcvar_err("setcvar", 
						      "%s (too many \
recursive par files", 
						      buffer);
				}
				
				if((fptrs[n_fp + 1] = fopen(name,"r"))==NULL)
					cv_setcvar_err("setcvar","cannot \
open par file %s",
						      name);

				n_fp++;
				continue;
			}
			else if( ! strncmp(line, "#!EndTable", 10))
			{
				/*
				 * close files and finish up
				 */

				while(n_fp) fclose(fptrs[n_fp--]);
				break;
			}
			else continue;
		}
		
		/*
		 *  first non-blank line has the columns
		 */
			
		if(! cv_ncolumns)

			cv_do_columns(line);

		else
			nrows += cv_do_rows(line, idx_fmt, nrows);
		
	}
	
	if(n_fp > 0) 
	{
		fclose(fptrs[n_fp]);
		n_fp--;
		goto loop;
	}
	
	return;
}


