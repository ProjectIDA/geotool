/*
 * NAME
 *	getpar
 *
 * DESCRIPTION
 *	retreive command line arguments.  Acquired from Caltech.
 *
 *	See getpar.3 for details.
 */
/* copyright (c) Robert W. Clayton
 *		 Seismological Laboratory
 *		 Caltech
 *		 Pasadena, CA 91125
 *
 * Getpar routines:
 *
 * Externally visible routines:
 *
 *		setpar(argc,argv)
 *		getpar(name,type,valptr)
 *		mstpar(name,type,valptr)
 *		endpar()
 *
 * To get C-version:
 *		cc -c getpar.c
 *
 * To get F77-version:
 * (obsolete, see note below - BGM)
 *		cp getpar.c fgetpar.c
 *		cc -c -DFORTRAN fgetpar.c
 *		rm fgetpar.c
 *
 * To get the environment processing stuff add the flag
 *-DENVIRONMENT to each of the cc's above.
 *
 *  Modification History:
 *  ---------------------
 *   5  2002    G. Klinkl       Added function for IDC CD receiver
 *   1  2000    B. MacRitchie   Added LIST_MISSING option for mstpar().
 *   10   99    B. MacRitchie   Rewrote Fortran interface as shells to
 *                              the C functions.  Removed FORTRAN define.
 *   6    95    Ronan Le Bras   "vs" option upgraded to handle LIST option.
 *   3    95	Warren Fox	added libpar_errno global variable
 *   6    94    Ronan Le Bras   Special treatment for vs option in gp_getvector.
 *                              No repetition factor allowed.
 *   4    94    Ethan Brown	Made separate driver for FORTRAN setpar.
 *   4    94    Ronan Le Bras   Added "vs" option in getpar to read
 *                              vectors of strings
 *   7    91    Jeff Given      Added getspar/mstspar: new functions interface
 *                              for string parameters (C only)
 *   6    91    Jeff Given      Added escape for new line in par files
 *  04/01/91    Jeff Given      Added parameter substitution capability
 *
 *  06/08/90	Glenn Nelson	(via Richard Stead) added "ENDPAR" command
 *				line feature, added fix for booleans in
 *				parfiles, added better way of obtaining
 *				environment, corrected spelling
 *  06/08/90	Richard Stead	cleaned up some of lint's complaints
 *  05/25/90	Doug Neuhauser	Changed arglist elements from pointers to
 *				integer offsets into ARGBUF to allow
 *				arbitrary realloc calls.
 *       	Rob Clayton	Original coding.
 *
 */

/* autoheader */
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include	<stdio.h>
#include        <stdlib.h>
#ifdef HAVE_STDARG_H
# include <stdarg.h>
#endif /* HAVE_STDARG_H */
#include        <string.h>

#include        <aesir.h>
#include        "libpar.h"

#define MAXLINE		LIBPAR_BUFSIZ	/* max length of line in par file */
#define MAXVALUE	LIBPAR_BUFSIZ	/* max length of value */
#define MAXNAME		128     /* max length of name */
#define MAXFILENAME	64	/* max length of par file name */
#define MAXVECTOR	10	/* max # of elements for unspecified vectors */
#define MAXPARLEVEL	8	/* max recursion level for par files */


#define INIT	 1	/* bits for FLAGS (ext_par.argflags) */
#define STOP	 2
#define LIST	 4
#define END_PAR	 8
#define VERBOSE	 16
#define LIST_MST 32

#define LISTINC		32	/* increment size for arglist */
#define BUFINC		LIBPAR_BUFSIZ	/* increment size for argbuf */

struct arglist		/* structure of list set up by setpar */
   {
	int argname_offset;
	int argval_offset;
	int hash;
   };

struct ext_par		/* global variables for getpar */
   {
	const char *progname;
	int argflags;
	struct arglist *arglist;
	struct arglist *arghead;
	char *argbuf;
	int nlist;
	int nbuf;
	size_t listmax;
	size_t bufmax;
	FILE *listout;
  }	ext_par;


/* abbreviations: */
#define AL 		struct arglist
#define PROGNAME	ext_par.progname
#define FLAGS		ext_par.argflags
#define ARGLIST		ext_par.arglist
#define ARGHEAD		ext_par.arghead
#define ARGBUF		ext_par.argbuf
#define NLIST		ext_par.nlist
#define NBUF		ext_par.nbuf
#define LISTMAX		ext_par.listmax
#define BUFMAX		ext_par.bufmax
#define LISTFILE	ext_par.listout


int  getpar_ (char *name, char *type, void *val, int lname, int ltype,
              int lval);
int  mstpar_ (char *name, char *type, void *val, int lname, int ltype,
              int lval);
void endpar_ (void);

static FILE *gp_create_dump (const char *fname, const char *filetype);
static void gp_add_entry (const char *name, char *value);
#ifdef ENVIRONMENT
static void gp_do_environment (const int ac, const char* const *av);
#endif
static void gp_subpar (const char **apl, char **apv, int start);
static void gp_do_par_file (const char *fname, int level);
static void gp_close_dump (FILE *file);
#ifdef HAVE_STDARG_H
static void gp_getpar_err (const char *subname, const char *format, ...);
#else
static void gp_getpar_err ();
#endif
static int  gp_compute_hash (const char *s);
static int  gp_getvector (char *list, char *type, void *val);
static char *gp_fgets (char *line, int maxline, FILE *file);

static const char *GP_UNKNOWN_PROG = "(unknown)";

int	libpar_errno;	/* exit status from libpar - atexit() may be
			 * used to capture control when libpar exits,
			 * see the example code below #ifdef TEST */

/* The next functions are added for the CD receiver */

int
setfpar (const char *progname, const char *file_name)
{
   FILE *fp;

   FLAGS= INIT;
   LISTFILE= stderr;

   PROGNAME = (char *)progname;
   ARGLIST= NULL;
   ARGBUF = NULL;
   NLIST= NBUF= LISTMAX= BUFMAX= 0;

   /* check if the file exists and is readable, using the standard error
      behavior is to complicated for the CD Receiver */

   if ((fp = fopen(file_name, "r")) == NULL)
      return(1);

   (void) fclose(fp);

   gp_do_par_file((char *)file_name,1);

   ARGHEAD= ARGLIST;
   return(0);
}

int
getparlistlen (void)
{
   return (NBUF);
}

int
getparline (int line, char **name, char **value)
{
   struct arglist *alptr;

   if (line < 0 || line >= NLIST)
      return (0);

   alptr = ARGLIST + line;
   *name = ARGBUF+alptr->argname_offset;
   *value = ARGBUF+alptr->argval_offset;
   return(1);
}

/* End added functions for CD receiver */

/* set up arglist & process INPUT command */
int
setpar (const int ac, const char **av)
{
	char *pn, *pv;
	const char *pl;
	char  t, name[MAXNAME], value[MAXVALUE];
	int   ivalue;
	FILE *file;
	int i, addflags, nevlist, endsetpar = 0;
	struct arglist *alptr;

        char *apv;
	const char  *apl;

        int argcount = ac;

	libpar_errno = 0;

	if((char **)av != (char **) NULL)
	{
		PROGNAME = *av;
	}
	else
	{
	        PROGNAME = GP_UNKNOWN_PROG;
		argcount = 0;
	}

	FLAGS= INIT;
	LISTFILE= stderr;

	ARGLIST= NULL;
	ARGBUF = NULL;
	NLIST= NBUF= LISTMAX= BUFMAX= 0;
#ifdef ENVIRONMENT
	gp_do_environment(argcount, av);
#endif
	nevlist= NLIST;
	while(--argcount > 0 && endsetpar == 0)
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
						gp_subpar(&apl, &apv, 0);
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
					gp_subpar(&apl, &apv, 0);
					pl = apl;
					pv = apv;
				}
				else *pv++ = *pl++;
		}
		*pv= '\0';
		if(name[0] == '-') gp_add_entry("SWITCH",&name[1]);
		else		gp_add_entry(name,value);
		if(strcmp("par",name)==0) /* par file */
			gp_do_par_file(value,1);

	/* Added by Glenn Nelson (nelson@ollie.UCSC.EDU) to allow mixture
	   of getpar() and ordinary command line stuff. */
		if (strcmp("ENDPAR", name) == 0) endsetpar = 1;
	   }

	ARGHEAD= ARGLIST;

#ifdef ENVIRONMENT
	if(getpar("NOENV","b",&ivalue)) ARGHEAD= ARGLIST+ nevlist;
#endif
	addflags= 0;
	if(getpar("STOP","b", &ivalue)) addflags |= STOP;
	if(getpar("VERBOSE","b", &ivalue)) addflags |= VERBOSE;
	*value= '\0';
	if(getpar("LIST_MST","s",value)) addflags |= LIST_MST;
	*value= '\0';
	if(getpar("LIST","s",value))
	   {
		addflags |= LIST;
		LISTFILE =gp_create_dump(value,"list");
	   }
	*value= '\0';
	if(getpar("INPUT","s",value))
	   {
		file =gp_create_dump(value,"list input");
		fprintf(file,"%s: getpar input listing\n",PROGNAME);
		for(i=0, alptr=ARGLIST; i<NLIST; i++, alptr++)
		   {
			fprintf(file,"%3d: %16s = %s\n",
				i,ARGBUF+alptr->argname_offset,
				ARGBUF+alptr->argval_offset);
		   }
		gp_close_dump(file);
	   }
	FLAGS |= addflags;

	/* Added by Glenn Nelson (nelson@ollie.UCSC.EDU) to allow setpar()
	   to terminate before all command line args are exhausted. */
	return argcount;
}

static void
gp_add_entry (const char *name, char *value)
{
	struct arglist *alptr;
	int len;
	char *ptr;
	char name2[MAXNAME], *pn, *apn;
	const char *pl, *apl;

        memset(name2, 0, sizeof(name2));

	if ((name == NULL) || ! strlen(name)) return;
	/*
	 * this function has been changed to allow substitutions
	 * on the left-hand side, ie, log-$(program)=$(logdir)/$(program)
	 */
	pn = name2;
	pl = name;

 	while (*pl)
		if(*pl == '$')
		{
			apl = pl;
			apn = pn;
			gp_subpar(&apl, &apn, 1);
			pl = apl;
			pn = apn;
		}
		else
			*pn++ = *pl++;
 	*pn = '\0';

	/* check arglist memory */
	if(NLIST >= (signed int)LISTMAX)
	   {
		LISTMAX += LISTINC;
		if(ARGLIST == NULL)
			ARGLIST= (AL *)malloc(LISTMAX * sizeof(AL));
		 else	ARGLIST= (AL *)realloc(ARGLIST,LISTMAX * sizeof(AL));
	   }
	/* check argbuf memory */
	len= strlen(name2) + strlen(value) + 2; /* +2 for terminating nulls */
	if(NBUF+len >= (signed int)BUFMAX)
	   {
		BUFMAX += BUFINC;
		if(ARGBUF == NULL)
			ARGBUF= (char *)malloc(BUFMAX);
		 else	ARGBUF= (char *)realloc(ARGBUF,BUFMAX);
	   }
	if(ARGBUF == NULL || ARGLIST == NULL)
		gp_getpar_err("setpar", "cannot allocate memory");

	/* add name */
	alptr= ARGLIST + NLIST;
	alptr->hash= gp_compute_hash(name2);
	alptr->argname_offset = NBUF;
	ptr= ARGBUF + NBUF;
	pn = name2;
	do *ptr++ = *pn; while(*pn++);

	/* add value */
	NBUF += len;
	alptr->argval_offset= ptr - ARGBUF;
	do *ptr++ = *value; while(*value++);
	NLIST++;
}

#ifdef ENVIRONMENT

#define BETTER_WAY	/* The environment is always available (as
			   suggested by Glenn Nelson (nelson@ollie.UCSC.EDU) */

#ifdef BETTER_WAY
	extern char     **environ;
#endif

static void
gp_do_environment (const int ac, const char * const *av)
   {
	char **ae;
	char *pl, *pn, *pv;
	char name[MAXNAME], value[MAXVALUE], t;

	/* The environ pointer ae, is assumed to have a specific relation
	   to the arg pointer av. This may not be portable. */
#ifndef BETTER_WAY
	ae= av +(ac+1);
	if(ae == NULL) return;
#else
	ae = environ;
#endif

	while(*ae != NULL)
	   {
		pl= *ae++;
		while(*pl == ' ' || *pl == '\t') pl++;
		/* get name */
		pn= name;
		while(*pl != '=' && *pl != '\0') *pn++ = *pl++;
		*pn = '\0';
		if(strcmp("NOENV",pn) == 0) return;

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
		gp_add_entry(name,value);
	   }
   }
#endif

void
endpar_ (void)
{
     endpar();
}

/* free arglist & argbuf memory, & process STOP command */
void
endpar (void)
{
	if(ARGLIST != NULL) free(ARGLIST);
	if(ARGBUF  != NULL) free(ARGBUF);
	ARGBUF=  NULL;
	ARGLIST= NULL;
	if(FLAGS & STOP)
	   {
		fprintf(stderr,"%s[endpar]: stop due to STOP in input\n",
			PROGNAME);
		libpar_errno=GETPAR_STOP;
		exit(GETPAR_STOP);
	   }
	if(FLAGS & LIST_MST)
	   {
		fprintf(stderr,"%s[endpar]: stop due to LIST_MST in input\n",
			PROGNAME);
		libpar_errno=GETPAR_STOP;
		exit(GETPAR_STOP);
	   }
	FLAGS= END_PAR;	/* this stops further getpar calls */
}

int
mstpar_ (char *name, char *type, void *val, int lname, int ltype, int lval)
{
     char  *name_c = NULL;
     char  *type_c = NULL;
     char  *val_c = NULL;
     int    i;
     int    status;

     if ((lname == 0) || (ltype == 0))
     {
	  gp_getpar_err("mstpar", "zero length argument");
	  return 0;
     }

     if ((name_c = UCALLOC (char, lname+1)) == NULL)
     {
	  gp_getpar_err("mstpar", "can't allocate memory");
	  return 0;
     }
     strncpy(name_c, name, (size_t)lname);
     name_c[lname] = '\0';

     /*
      * probably don't need to allocate a new area if type is always
      * 1 or 2 chars, but let's do it anyway
      */
     if ((type_c = UCALLOC (char, ltype+1)) == NULL)
     {
	  UFREE (name_c);
	  gp_getpar_err("mstpar", "can't allocate memory");
	  return 0;
     }
     strncpy(type_c, type, (size_t)ltype);
     type_c[ltype] = '\0';

     /* the following line corrects a common input error */
     if (ltype > 1)
	  if(type_c[1]=='v') { type_c[1]= type_c[0]; type_c[0]='v'; }

     if (type[0] != 's')
     {
	  status = mstpar (name_c, type_c, val);
     }
     else
     {
	  /*
	   * if this is a string, allocate a new space,
	   * C needs one extra byte for the NULL terminator
	   */
	  if (lval == 0)
	  {
	       UFREE (name_c);
	       UFREE (type_c);
	       gp_getpar_err("mstpar", "zero length output argument");
	       return 0;
	  }

	  if ((val_c = UCALLOC (char, lval+1)) == NULL)
	  {
	       UFREE (name_c);
	       UFREE (type_c);
	       gp_getpar_err("mstpar", "can't allocate memory");
	       return 0;
	  }
	  status = mstpar (name_c, type_c, val_c);

	  if ((i = strlen(val_c)) > 0)
	  {
	       /*
		* if successful, copy
		*/
            strncpy((char *)val, val_c, (size_t)lval);
	  }
	  /*
	   * pad with spaces
	   */
	  while (i < lval)
	       ((char*)val)[i++] = ' ';
     }

     UFREE (val_c);
     UFREE (name_c);
     UFREE (type_c);
     return (status);
}

int
mstpar (const char *name, const char *type, void *val)
{
     int cnt;
     size_t ltype;
     char *type_c;
     const char *typemess;

     if(FLAGS & END_PAR)
	  gp_getpar_err("mstpar","called after endpar");
     if( (FLAGS & INIT) == 0)
	  gp_getpar_err("mstpar","not initialized with setpar");

     if( (cnt= getpar(name,type,val)) > 0) return(cnt);

     ltype = strlen(type);
     if ((type_c = UCALLOC (char, ltype+1)) == NULL)
     {
	    gp_getpar_err("mstpar", "can't allocate memory");
	    return 0;
     }
     strncpy(type_c, type, (size_t)ltype);
     type_c[ltype] = '\0';

     /* The following line corrects a common input error */
     if(type_c[1]=='v') { type_c[1]= type_c[0]; type_c[0]='v'; }

     switch(*type_c)
     {
     case 'd': typemess= "an integer"; break;
     case 'f': typemess= "a float";    break;
     case 'F': typemess= "a double";   break;
     case 's': typemess= "a string";   break;
     case 'b': typemess= "a boolean";  break;
     case 'v':
	  switch(type_c[1])
	  {
	  case 'd': typemess= "an integer vector";      break;
	  case 'f': typemess= "a float vector";         break;
	  case 'F': typemess= "a double vector";        break;
	  case 's': typemess= "a vector of strings";    break;
	  default : typemess= "unknown vector (error)"; break;
	  }
	  break;
     default : typemess= "unknown (error)"; break;
     }
     /* print error, exit if LIST_MST not set */
     if(FLAGS & LIST_MST)
     {
	  fprintf (stderr, "mstpar: must specify value for '%s', expecting %s\n",
		   name,typemess);
     }
     else
     {
	  gp_getpar_err("mstpar", "must specify value for '%s', expecting %s",
			name,typemess);
     }

     UFREE (type_c);

     return 0;
}

int
getpar_ (char *name, char *type, void *val, int lname, int ltype, int lval)
{
     char  *name_c = NULL;
     char  *type_c = NULL;
     char  *val_c = NULL;
     int    i;
     int    status;

     if ((lname == 0) || (ltype == 0))
     {
	  gp_getpar_err("getpar", "zero length argument");
     }

     if ((name_c = UCALLOC (char, lname+1)) == NULL)
     {
	  gp_getpar_err("getpar", "can't allocate memory");
     }
     strncpy(name_c, name, (size_t)lname);
     name_c[lname] = '\0';

     /*
      * probably don't need to allocate a new area if type is always
      * 1 or 2 chars, but let's do it anyway
      */
     if ((type_c = UCALLOC (char, ltype+1)) == NULL)
     {
	  UFREE (name_c);
	  gp_getpar_err("getpar", "can't allocate memory");
     }
     strncpy(type_c, type, (size_t)ltype);
     type_c[ltype] = '\0';

     /* the following line corrects a common input error */
     if (ltype > 1)
	  if(type_c[1]=='v') { type_c[1]= type_c[0]; type_c[0]='v'; }

     if (type_c[0] != 's')
     {
	  status = getpar (name_c, type_c, val);
     }
     else
     {
	  /*
	   * if this is a string, allocate a new space,
	   * C needs one extra byte for the NULL terminator
	   */
	  if (lval == 0)
	  {
	       UFREE (name_c);
	       UFREE (type_c);
	       gp_getpar_err("getpar", "zero length output argument");
	  }

	  if ((val_c = UCALLOC (char, lval+1)) == NULL)
	  {
	       UFREE (name_c);
	       UFREE (type_c);
	       gp_getpar_err("getpar", "can't allocate memory");
	  }
	  status = getpar (name_c, type_c, val_c);

	  if ((i = strlen(val_c)) > 0)
	  {
	       /*
		* if successful, copy
		*/
               strncpy((char *)val, val_c, (size_t)lval);
	  }
	  /*
	   * pad with spaces
	   */
	  while (i < lval)
	       ((char*)val)[i++] = ' ';
     }

     UFREE (val_c);
     UFREE (name_c);
     UFREE (type_c);
     return (status);
}

int
getpar (const char *name, const char *type, void *val)
{
	char *sptr;
	struct arglist *alptr;
	double *dbl;
	float *flt;
	int   *ival;
	int h, hno, hyes, found;
	size_t ltype;
	char line[MAXLINE], *str, *noname, *type_c;
	double	bc_result;
	int	bc;

	flt  = (float *) val;
	ival = (int *)   val;
	dbl  = (double *)val;

	if(FLAGS & END_PAR)
		gp_getpar_err("getpar","called after endpar");
	if( (FLAGS & INIT) == 0)
		gp_getpar_err("getpar","not initialized with setpar");
	if(FLAGS & VERBOSE)
		fprintf(stderr,"getpar: looking for %s\n",name);

	found=0;

	if(NLIST <= 0) return(found);

	ltype = strlen(type);
	if ((type_c = UCALLOC (char, ltype+1)) == NULL)
	{
	    gp_getpar_err("getpar", "can't allocate memory");
	    return 0;
	}
	strncpy(type_c, type, (size_t)ltype);
	type_c[ltype] = '\0';

	/* The following line corrects a common input error */
	if(type_c[1]=='v') { type_c[1]= type_c[0]; type_c[0]='v'; }

	/*
	 *  if list is NULL then return NOW; the following "for" loop
	 *  the pointer ARGLIST + (NLIST-1) gives ARGLIST -1 which
	 *  does not test to < ARGHEAD for some reason
         */

	if(*type_c == 'b') goto boolean;

	h= gp_compute_hash(name);

	/* search list backwards, stopping at first find */
	for(alptr= ARGLIST +(NLIST-1); alptr >= ARGHEAD; alptr--)
	   {
		if(alptr->hash != h) continue;
		if(strcmp(ARGBUF+alptr->argname_offset,name) != 0) continue;
		str= ARGBUF + alptr->argval_offset;

	        /* check for "bc <expression>" (version 103.2) */
	        if ( strncmp( str, "bc", 2 ) == 0 &&
		     (*type_c == 'd' || *type_c == 'f' || *type_c == 'F') )
		{
			FILE *fp;
			char cmd[200];
			sprintf( cmd, "echo %s | bc -l", str+2 );
			fp = popen( cmd, "r" );
			if (fscanf( fp, "%lf", &bc_result ) != 1) return 0;
			pclose(fp);
			bc = 1;		/* bc in effect for this par */
		}
		else
			bc = 0;

		switch(*type_c)
		   {
			case 'd':
			        ival= (int *) val;
				if (bc)
					*ival = (int) bc_result;
				else
					*ival= atoi(str);
				found=1;
				break;
			case 'f':
				flt= (float *) val;
				if (bc)
					*flt = (float) bc_result;
				else
					*flt= atof(str);
				found=1;
				break;
			case 'F':
				dbl= (double *) val;
				if (bc)
					*dbl = bc_result;
				else
					*dbl= atof(str);
				found=1;
				break;
			case 's':
                                sptr= (char *) val;
                                while(*str) *sptr++ = *str++;
                                *sptr= '\0';
                                found=1;
                                break;
			case 'v':
				found= gp_getvector(str,type_c,val);
				break;
			default:
				gp_getpar_err("getpar",
					"unknown conversion type %s",type_c);
				break;
		   }
		break;
	   }
	goto list;

boolean:

	noname= line;
	sprintf(noname,"no%s",name);
	hno = gp_compute_hash(noname);
	hyes= gp_compute_hash(  name);
	found=0;
	/* search list backwards, stopping at first find */
	for(alptr= ARGLIST +(NLIST-1); alptr >= ARGHEAD; alptr--)
	   {
		if(alptr->hash != hno && alptr->hash != hyes) continue;
		if(strcmp(ARGBUF+alptr->argname_offset,  name)== 0)
		   {
			if(*(ARGBUF+alptr->argval_offset) == '\0') *ival= 1;
			 else *ival= (int)atol(ARGBUF+alptr->argval_offset);
			found++;
			break;
		   }
		if(strcmp(ARGBUF+alptr->argname_offset,noname)== 0)
		   {	*ival= 0; found++; break; }
	   }
   list:
	if(FLAGS & LIST)
	   {
		switch(*type_c)
		   {
			case 'd': sprintf(line,"(int) = %d", *ival); break;
			case 'f': flt= (float *)val;
				  sprintf(line,"(flt) = %14.6e",*flt); break;
			case 'F': dbl= (double *)val;
				  sprintf(line,"(dbl) = %14.6e",*dbl); break;
                        case 's': sprintf(line,"(str) = %s",(char *) val); break;
			case 'b': sprintf(line,"(boo) = %d",* ival); break;
			case 'v': switch(type_c[1])
				   {
					/* should list these out */
					case 'd': sprintf(line,"(int vec)");
						break;
					case 'f': sprintf(line,"(flt vec)");
						break;
					case 'F': sprintf(line,"(dbl vec)");
						break;
					case 's': sprintf(line,"(str vec)");
						break;
					default : sprintf(line," vec type error");
						break;
				   }
				  break;
			default : sprintf(line," type error"); break;
		   }
		fprintf(LISTFILE,"%16s (%s) %s \n",name,
			(found ? "set":"def"),line);
	   }
        UFREE (type_c);
	return(found);
}

static FILE*
gp_create_dump (const char *fname, const char *filetype)
{
	FILE *temp;

	if(*fname == '\0') return(stderr);
	if(strcmp(fname,"stderr") == 0) return(stderr);
	if(strcmp(fname,"stdout") == 0) return(stdout);
	if( (temp= fopen(fname,"w")) != NULL) return(temp);
	fprintf(stderr,"%s[setpar]: cannot create %s file %s\n",
		PROGNAME,filetype,fname);
	return(stderr);
}

static void
gp_close_dump (FILE *file)
{
	if(file == stderr || file == stdout) return;
	fclose(file);
}

static int
gp_compute_hash (const char *s)
{
	int h;
	h= s[0];
	if(s[1]) h |= (s[1])<<8;	else return(h);
	if(s[2]) h |= (s[2])<<16;	else return(h);
	if(s[3]) h |= (s[3])<<24;
	return(h);
}

static void
gp_do_par_file (const char *fname, int level)
{
	const char *pl;
	char *pn, *pv;
	char t, line[MAXLINE], name[MAXNAME], value[MAXVALUE];
	FILE *file;
	const char *apl;
	char *apv;

	if(level > MAXPARLEVEL)
		gp_getpar_err("setpar","%d (too many) recursive par file",level);

	if(*fname == '\0') return;

	if( (file=fopen(fname,"r"))==NULL)
		gp_getpar_err("setpar","cannot open par file %s",fname);

	while( gp_fgets(line,MAXLINE,file) != NULL )
	   {
		pl= line;
		/* loop over entries on each line */
	loop:	while(*pl==' ' || *pl=='\t') pl++;
		if(*pl=='\0'|| *pl=='\n') continue;
		if(*pl=='#') continue; /* comments on rest of line */

		/* get name */
		pn= name;
		while(*pl != '=' && *pl != '\0' && *pl != ' '
			&& *pl != '\n'		/* FIX by Glenn Nelson */
			&& *pl != '\t')
                {
                   if ((pn - name) < (MAXNAME - 1)) /* FIX by Gerald Klinkl */
                      *pn++ = *pl++;
                   else
                     gp_getpar_err("gp_do_par_file",
                                   "name [%.63s] \n\t>%d bytes", name,
                                   MAXNAME - 1);
                }
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
						gp_subpar(&apl, &apv, 0);
						pl = apl;
						pv = apv;
					}
                                        /* FIX by Gerald Klinkl */
					else if ((pv - value) < (MAXVALUE - 1))
                                           *pv++ = *pl++;
                                        else
                                           gp_getpar_err("gp_do_par_file",
                                              "value for [%.63s] >%d bytes]",
                                               name, MAXVALUE);
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
					gp_subpar(&apl, &apv, 0);
					pl = apl;
					pv = apv;
				}
                                /* FIX by Gerald Klinkl */
				else if ((pv - value) < (MAXVALUE - 1))
                                   *pv++ = *pl++;
                                else
                                   gp_getpar_err("gp_do_par_file",
                                                 "value for [%.63s] >%d bytes",
                                                 name, MAXVALUE);
		}
		*pv= '\0';

		gp_add_entry(name,value);
		if(strcmp("par",name) == 0)
			gp_do_par_file(value,level+1);
		goto loop;
	   }
	fclose(file);
   }

#ifdef HAVE_STDARG_H
static void
gp_getpar_err (const char *subname, const char *format, ...)
#else
static void
gp_getpar_err (va_alist) va_dcl
#endif
{
	va_list va;

#ifdef HAVE_STDARG_H
	va_start(va, format);
#else
	char *format;
	const char *subname;
	va_start(va);
	subname = va_arg(va, char *);
	format = va_arg(va, char *);
#endif
	if(format == NULL) return;

	fprintf(stderr,"\n***** ERROR in %s[%s] *****\n\t",
		(PROGNAME == NULL ? "(unknown)" : PROGNAME),subname);
	vfprintf(stderr, format, va);
	fprintf(stderr,"\n");
	libpar_errno=GETPAR_ERROR;
	exit(GETPAR_ERROR);
}

static int
gp_getvector (char *list, char *type, void *val)
{
	char *p        ;
	int index, cnt ;
	char *valptr            ;
	int limit               ;
	int ival, *iptr, lstr   ;
	float fval, *fptr       ;
	double dval, *dptr      ;
	char **sptr             ;
	const char *sep=","     ;

	limit= MAXVECTOR;
	if(type[2] == '(' || type[2] == '[') limit= (int)atol(&type[3]);
	if(limit <= 0)
		gp_getpar_err("getpar","bad limit=%d specified",limit);
	index= 0;
	p= list;
	while(*p != '\0'  && index < limit)
	   {
		cnt=1;
	 backup: /* return to here if we find a repetition factor */
		while(*p == ' ' || *p == '\t') p++;
		if(*p == '\0') return(index);
		valptr= p;
		if(type[1] != 's')
		  {
		      while( *p != ',' && *p != '*' && *p != 'x' && *p != 'X' &&
			    *p != '\0') p++;
		      if(*p == '*' || *p == 'x' || *p == 'X')
			{
			    cnt= (int)atol(valptr);
			    if(cnt <= 0)
			      gp_getpar_err("getpar",
					    "bad repetition factor=%d specified",
					    cnt);
			    if(index+cnt > limit) cnt= limit - index;
			    p++;
			    goto backup;
			}
		  }
		else if(type[1] == 's')
		  {
		      while( *p != ',' && *p != '\0') p++ ;
		  }
		switch(type[1])
		  {
		  case 'd':
		    iptr= (int *) val;
		    ival= (int)atol(valptr);
		    while(cnt--) iptr[index++] = ival;
		    break;
		  case 'f':
		    fptr= (float *) val;
		    fval= atof(valptr);
		    while(cnt--) fptr[index++] = fval;
		    break;
		  case 'F':
		    dptr= (double *) val;
		    dval= atof(valptr);
		    while(cnt--) dptr[index++] = dval;
		    break;
		  case 's':
		    sptr= (char **) val;
		    while(cnt--)
		      {
			  lstr = strcspn(valptr,sep) ;
			  strncpy(sptr[index++],valptr,(size_t)lstr) ;
			  sptr[index-1][lstr] = '\0' ;
		      }
		    break;
		default:
				gp_getpar_err("getpar",
					"bad vector type=%c specified",type[1]);
				break;
		   }
		if(*p != '\0') p++;
	   }
	return(index);
}

/*  This allows parameter substitution */
static void
gp_subpar (const char **apl, char **apv, int start)
{
	const char *pl;
	char *pv;
	char     subname[MAXNAME];
	int      valid = 0;
	const char *bpl;
	char *bpv;

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
				gp_subpar(&bpl, &bpv, 0);
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
			ARGHEAD= ARGLIST;
			if(getpar(subname, "s", pv))
			{
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
gp_fgets( char *line, int maxline, FILE *file)
{
	char *p;
	int i;
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

char *
mstspar (const char *parname)
{
	char parvalue[MAXVALUE];

	mstpar(parname, "s", parvalue);

	return(strdup(parvalue));
}

char *
getspar (const char *parname, const char *defvalue)
{
	char parvalue[MAXVALUE];

	parvalue[0] = '\0';

	if(defvalue)
	{
	    strncpy(parvalue, defvalue, MAXVALUE);
	    parvalue[MAXVALUE-1] = '\0';
	}

	getpar(parname, "s", parvalue);

	if(! defvalue && parvalue[0] == '\0') return( (char *) 0 );

	else return(strdup(parvalue));

}


#ifdef TEST
/* test code for the new libpar_errno variable */

/*
 * % cc -DTEST -I../../include getpar.c -o testlibpar libpar.a
 * % testlibpar  	(echo $status gives 0)
 * % testlibpar one=2	(echo $status gives 0)
 * % testlibpar STOP	(echo $status gives 101)
 * % testlibpar par=/z	(echo $status gives 100)
 */

/* callback function registered with atexit() */
static void
libpar_exit (void)
{
	/* if libpar called exit, the libpar_errno will be set */
	if ( libpar_errno )
		printf("caught error %i in libpar\n", libpar_errno);
}

int
main (const int argc, const char **argv)
{
	int	one = 1, two = 2;	/* dummy variables */
	atexit(libpar_exit);		/* set callback for libpar error */
	setpar(argc,argv);
	getpar("one","d",&one);
	getpar("two","d",&two);
	endpar();

	printf ("one=%d\n", one);
	printf ("two=%d\n", two);
	return 0;
}
#endif
