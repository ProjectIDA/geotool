/*
 * NAME
 *	getarg
 *
 * DESCRIPTION
 *	get subroutine arguments from a string.  Acquired from Caltech.
 *
 *	See getarg.3 for details.
 *
 * AUTHOR
 * copyright (c) Robert W. Clayton
 *		 Seismological Laboratory
 *		 Caltech
 *		 Pasadena, CA 91125
 *
 * 28 Oct 1999  Bonnie MacRitchie  Rewrote Fortran interface as shells to
 *                                 the C functions.  Removed FORTRAN define.
 *
 * 24 Jul 1991  J Given         Added function getsarg() for dynamic
 *                              string allocation
 *
 * 25 Apr 1991	Cynde K. Smith	Added vector of string capability for C
 *			       	version only.
 * 26 Apr 1991	Cynde K. Smith  Replaced calls to atof and atoi with
 *			       	strtod and strtol to allow error checking.
 * 09 May 1991  Cynde K. Smith  Added countarg() and cntarg_() subroutines.
 * 14 May 1991  Cynde K. Smith  Added lenarg() subroutine.
 *
 *
 * Getarg routines:
 *
 * Externally visable routines:
 *
 *		setarg(argc,argv)
 *		countarg(name,type)
 *		lenarg(name)
 *		getarg(name,type,valptr)
 *		endarg()
 *
 * To get C-version:
 *		cc -c getarg.c
 *
 * To get F77-version:
 * (obsolete, now automatically get Fortran interface - BGM)
 *		cp getarg.c fgetarg.c
 *		cc -c -DFORTRAN fgetarg.c
 *		rm fgetarg.c
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
#include	<math.h>
#include	<string.h>

#include        "aesir.h"
#include        "libpar.h"

#define MAXVALUE	LIBPAR_BUFSIZ	/* max length of value */
#define MAXNAME		64	/* max length of name */
#define MAXVECTOR	10	/* max # of elements for unspecified vectors */
#define GETARG_ERROR	-1	/* error status for getarg error */

#define INIT	 1	/* bits for FLAGS (ext_arg.argflags) */
#define END_PAR	 2

#define LISTINC		32	/* increment size for arglist */
#define BUFINC		LIBPAR_BUFSIZ	/* increment size for argbuf */


struct arglist		/* structure of list set up by setarg */
   {
	char *argname;
	char *argval;
	int hash;
   };
struct ext_arg		/* global variables for getarg */
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
   }	ext_arg;

/* abbreviations: */
#define AL 		struct arglist
#define PROGNAME	ext_arg.progname
#define FLAGS		ext_arg.argflags
#define ARGLIST		ext_arg.arglist
#define ARGHEAD		ext_arg.arghead
#define ARGBUF		ext_arg.argbuf
#define NLIST		ext_arg.nlist
#define NBUF		ext_arg.nbuf
#define LISTMAX		ext_arg.listmax
#define BUFMAX		ext_arg.bufmax


void  setarg_ (const char *list, const char *subname, int llist, int lsubname);
void  endarg_ (void);
int   cntarg_ (const char *name, const char *type, int lname, int ltype);
int   fgtarg_ (const char *name, const char *type, void *val, int lname,
               int ltype, int lval);

static int  ga_compute_hash (const char *s);
static int  ga_getvector (const char *list, const char *type, char *val);
#ifdef HAVE_STDARG_H
static int ga_getarg_err (const char *subname, const char *format, ...);
#else
static int ga_getarg_err ();
#endif
static void ga_add_entry (const char *name, const char *value);

void
setarg_ (const char *list, const char *subname, int llist, int lsubname)
{
     char  *subname_c = NULL;
     char  *list_c = NULL;

     if (llist == 0)
	  return;

     if ((list_c = UCALLOC (char, llist+1)) == NULL)
	  return;

     strncpy(list_c, list, (size_t)llist);
     list_c[llist] = '\0';

     if (lsubname > 0)
     {
	  if ((subname_c = UCALLOC (char, lsubname+1)) == NULL)
	  {
	       UFREE (list_c);
	       return;
	  }
	  strncpy(subname_c, subname, (size_t)lsubname);
	  subname_c[lsubname] = '\0';
     }

     setarg (list_c, subname_c);

     UFREE (list_c);

#if 0 /* don't free this as it is assigned to a global variable,
       * it is freed by endarg()
       */
     UFREE (subname_c);
#endif
     return;
}

void
setarg (const char *list, const char *subname)
{
	char *pn, *pv;
	const char *pl;
	int i;
	int lenlist;
	char t, name[MAXNAME], value[MAXVALUE];


	PROGNAME = subname;
	FLAGS = INIT;

	ARGLIST = NULL;
	ARGBUF = NULL;
	NLIST = NBUF = LISTMAX= BUFMAX= 0;

	if(list == NULL) return;

	lenlist = strlen(list);
	if(lenlist == 0) return;

	pl = list;
	/* loop over entries on each line */

	for(i=0; i<lenlist && *pl != '\0'; i++)
	{
		while(*pl==' ' || *pl=='\t') pl++;
		if(*pl=='\0'|| *pl=='\n') continue;

		/* get name */
		pn= name;
		while(*pl != '=' && *pl != '\0' && *pl != ' '
		      && *pl != '\t') *pn++ = *pl++;
		*pn = '\0';
		if(*pl == '=') pl++;


		/* get value */

		*value= '\0';
		pv= value;
		if(*pl == '\"' || *pl == '\'')
		{
			t = *pl++;
			while(*pl != '\0')
			{
				if(*pl == t)
				{
					   if (pl[-1] != '\\' &&
					       (pl[1] == ' ' || pl[1] == '\0'))
					     {
					       pl++;
					       break;
					     }
				}
				*pv++ = *pl++;
			}
		}
		else
		{
			while(*pl && *pl != ' ' && *pl != '\t')
				*pv++ = *pl++;
		}

		*pv= '\0';
		ga_add_entry(name,value);
	}
}

/* add an entry to arglist, expanding memory */
static void
ga_add_entry (const char *name, const char* value)
{
	struct arglist *alptr;
	int len;
	char *ptr;

	/*fprintf(stderr,"getarg: adding %s (%s)\n",name,value);*/
	/* check arglist memory */
	if(NLIST >= (signed int)LISTMAX)
	   {
		LISTMAX += LISTINC;
		if(ARGLIST == NULL)
			ARGLIST= (AL *)malloc(LISTMAX * sizeof(AL));
		 else	ARGLIST= (AL *)realloc(ARGLIST,LISTMAX * sizeof(AL));
	   }
	/* check argbuf memory */
	len= strlen(name) + strlen(value) + 2; /* +2 for terminating nulls */
	if(NBUF+len >= (signed int)BUFMAX)
	   {
		BUFMAX += BUFINC;
		if(ARGBUF == NULL)
			ARGBUF= (char *)malloc(BUFMAX);
		 else	ARGBUF= (char *)realloc(ARGBUF,BUFMAX);
	   }
	if(ARGBUF == NULL || ARGLIST == NULL)
	   {
                ga_getarg_err("setarg","cannot allocate memory");
		return;
	   }

	/* add name */
	alptr= ARGLIST + NLIST;
	alptr->hash= ga_compute_hash(name);
	ptr= alptr->argname= ARGBUF + NBUF;
	do *ptr++ = *name; while(*name++);

	/* add value */
	NBUF += len;
	alptr->argval= ptr;
	do *ptr++ = *value; while(*value++);
	NLIST++;
}

void
endarg_ (void)
{
     endarg();
     /* UFREE (PROGNAME); */
     return;
}

/* free arglist & argbuf memory, & process STOP command */
void
endarg (void)
{
	if(ARGLIST != NULL) free(ARGLIST);
	if(ARGBUF  != NULL) free(ARGBUF);
	ARGBUF=  NULL;
	ARGLIST= NULL;
	FLAGS= END_PAR;	/* this stops further getarg calls */
}

/* count the number of arguments for a particular parameter name */
int
cntarg_ (const char *name, const char *type, int lname, int ltype)
{
     char  *name_c = NULL;
     char  *type_c = NULL;
     int    status;

     if ((lname == 0) || (ltype == 0))
	  return ga_getarg_err("cntarg","zero length name or type");

     if ((name_c = UCALLOC (char, lname+1)) == NULL)
	  return ga_getarg_err("cntarg","can't alloc memory");

     strncpy (name_c, name, (size_t)lname);
     name_c[lname] = '\0';

     if ((type_c = UCALLOC (char, ltype+1)) == NULL)
     {
	  UFREE (name_c);
	  return ga_getarg_err("cntarg","can't alloc memory");
     }

     status = countarg (name_c, type_c);
     UFREE (name_c);
     UFREE (type_c);
     return (status);
}

int
countarg (const char *name, const char *type)
{
	int 	found, h;
	char 	*str, *ptr;
	struct arglist *alptr;

	if (FLAGS & END_PAR)
	{
		return ga_getarg_err("countarg","called after endarg");
	}

	if ((FLAGS & INIT) == 0)
	{
		return ga_getarg_err("countarg","not initialized with setarg");
	}


	if (NLIST == 0 || ARGLIST == NULL)
	{
		return (0);
	}


	found=0;

	h = ga_compute_hash(name);

	/*
	 *  if list is NULL then return NOW; the following "for" loop
	 *  the pointer ARGLIST + (NLIST-1) gives ARGLIST -1 which
	 *  does not test to < ARGHEAD for some reason
         */

	if (NLIST <= 0)
	{
		return (0);
	}


	/* search list backwards, stopping at first find */
	for (alptr = ARGLIST +(NLIST-1); alptr >= ARGLIST; alptr--)
	{
		if (alptr->hash != h)
			continue;
		if (strcmp(alptr->argname, name) != 0)
			continue;
		str = alptr->argval;
		ptr = str;

		switch (*type)
		{
	       	case 'd':
		case 'f':
		case 'F':
			/*
			 * Count the number of commas to detemine list
			 * size.  If the str isn't NULL than there
			 * is at least one element and at least one
			 * more than there are commas.
			 */
			while ((ptr = strchr(ptr, ',')) != NULL)
			{
					found++;
					ptr++;
			}
			if (str != NULL)
				found++;
			break;
	       	case 's':
			while ((ptr = strchr(ptr, ',')) != NULL)
			{
				if (ptr[-1] != '\\')
					found++;
				ptr++;
			}
			if (str != NULL)
				found++;
			break;
	       	default:
		       	return ga_getarg_err("countarg",
		       		      "unknown conversion type %s",type);
		       	break;
		}
		break;
	}
	return (found);
}

int
lenarg (const char *name)
{
	int 	h, len = 0, new_len;
	char 	*str, *ptr1, *ptr2;
	struct arglist *alptr;

	if (FLAGS & END_PAR)
	{
		return ga_getarg_err("lenarg","called after endarg");
	}

	if ((FLAGS & INIT) == 0)
	{
		return ga_getarg_err("lenarg","not initialized with setarg");
	}

	if (NLIST == 0 || ARGLIST == NULL)
	{
		return (0);
	}


	h = ga_compute_hash(name);

	/*
	 *  if list is NULL then return NOW; in the following "for"
	 *  loop the pointer ARGLIST + (NLIST-1) gives ARGLIST -1
	 *  which does not test to < ARGHEAD for some reason
         */

	if (NLIST <= 0)
	{
		return (0);
	}


	/* search list backwards, stopping at first find */
	for (alptr = ARGLIST +(NLIST-1); alptr >= ARGLIST; alptr--)
	{
		if (alptr->hash != h)
			continue;
		if (strcmp(alptr->argname, name) != 0)
			continue;
		str = alptr->argval;
		ptr1 = str;
		len = new_len = 0;
	       	while ((ptr2 = strchr(ptr1, ',')) != NULL)
	       	{
	       		new_len += ptr2 - ptr1;
			if (ptr2[-1] != '\\')
			{
				if (new_len > len)
					len = new_len;
				new_len = 0;
			}
			ptr2++;
			ptr1 = ptr2;
		}

		if (ptr1)
			new_len = strlen(ptr1);
		if (new_len > len)
			len = new_len;

		break;
	}
	return (len);
}



int
fgtarg_ (const char *name, const char *type, void *val, int lname,
         int ltype, int lval)
{
     char  *name_c = NULL;
     char  *type_c = NULL;
     char  *val_c = NULL;
     int    i;
     int    status;

     if ((lname == 0) || (ltype == 0))
     {
	  return ga_getarg_err("fgtarg", "zero length argument");
     }

     /*
      * for now, it's an error to get a Fortran vector of strings
      */
     if ((ltype > 1) && (type[0] == 'v') && (type[1] == 's'))
	  return ga_getarg_err("fgtarg", "fortran string vector");

     if ((name_c = UCALLOC (char, lname+1)) == NULL)
     {
	  return ga_getarg_err("fgtarg", "can't allocate memory");
     }
     strncpy (name_c, name, (size_t)lname);
     name_c [lname] = '\0';

     /*
      * probably don't need to allocate a new area if type is always
      * 1 or 2 chars, but let's do it anyway
      */
     if ((type_c = UCALLOC (char, ltype+1)) == NULL)
     {
	  UFREE (name_c);
	  return ga_getarg_err("fgtarg", "can't allocate memory");
     }
     strncpy (type_c, type, (size_t)ltype);
     type_c [ltype] = '\0';

     /* the following line corrects a common input error */
     if (ltype > 1)
	  if(type_c[1]=='v') { type_c[1]= type_c[0]; type_c[0]='v'; }

     if (type[0] != 's')
     {
	  status = getarg (name_c, type_c, val);
     }
     else
     {
	  if (lval == 0)
	  {
	       UFREE (name_c);
	       UFREE (type_c);
	       return ga_getarg_err("fgtarg",
				    "zero length output argument");
	  }

	  /*
	   * if this is a string, allocate a new space,
	   * C needs one extra byte for the NULL terminator
	   */
	  if ((val_c = UCALLOC (char, lval+1)) == NULL)
	  {
	       UFREE (name_c);
	       UFREE (type_c);
	       return ga_getarg_err("fgtarg", "can't allocate memory");
	  }
	  status = getarg (name_c, type_c, val_c);

	  if ((i = strlen(val_c)) > 0)
	  {
	       /*
		* if successful, copy
		*/
            strncpy (val, val_c, (size_t)lval);
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
getarg (const char *name, const char *type, void *val)
{
	char *sptr;
	struct arglist *alptr;
	double *dbl;
	int   *ival;
	float *flt;
        size_t ltype;
	int h, hno, hyes, found;
	char noname[MAXNAME+2], *str, *ptr, *type_c;

	dbl = (double *)val;
	ival = (int *)val;
	flt = (float *)val;

	/*fprintf(stderr,"looking for %s, type=%s\n",name,type);*/
	if(FLAGS & END_PAR)
		return ga_getarg_err("getarg","called after endarg");
	if( (FLAGS & INIT) == 0)
		return ga_getarg_err("getarg","not initialized with setarg");
	if (val == NULL)
		return ga_getarg_err("getarg", "NULL pointer value");

	if(NLIST == 0 || ARGLIST == NULL) return(0);

	ltype = strlen(type);
	if ((type_c = UCALLOC (char, ltype+1)) == NULL)
	{
	    return ga_getarg_err("getarg", "can't allocate memory");
	}
	strncpy (type_c, type, (size_t)ltype);
	type_c [ltype] = '\0';

	/* The following line corrects a common input error */
	if(type_c[1]=='v') { type_c[1]= type_c[0]; type_c[0]='v'; }

	found=0;

	if(*type_c == 'b') goto boolean;

	h = ga_compute_hash(name);

	/*
	 *  if list is NULL then return NOW; the following "for" loop
	 *  the pointer ARGLIST + (NLIST-1) gives ARGLIST -1 which
	 *  does not test to < ARGHEAD for some reason
         */

	if(NLIST <= 0) {
	    UFREE (type_c);
	    return(found);
	}

	/* search list backwards, stopping at first find */
	for(alptr= ARGLIST +(NLIST-1); alptr >= ARGLIST; alptr--)
	   {
		/*fprintf(stderr,"getarg: checking %s\n",alptr->argname);*/
		if(alptr->hash != h) continue;
		if(strcmp(alptr->argname,name) != 0) continue;
		str= alptr->argval;
		switch(*type_c)
		   {
			case 'd':
				*ival= (int) strtol(str, &ptr, 0);
				if (ptr == str)
					found = 0;
				else
					found = 1;
				break;
			case 'f':
				flt= (float *) val;
				*flt= (float) strtod(str, &ptr);
				if (ptr == str)
					found = 0;
				else
					found = 1;
				break;
			case 'F':
				dbl= (double *) val;
				*dbl= strtod(str, &ptr);
				if (ptr == str)
					found = 0;
				else
					found = 1;
				break;
			case 's':
                                sptr= (char *) val;
                                while(*str) *sptr++ = *str++;
                                *sptr= '\0';
                                found=1;
                                break;
			case 'v':
                                if ((found = ga_getvector(str,type_c,(char *)val)) ==
				    GETARG_ERROR)
					return found;
				break;
			default:
				return ga_getarg_err("getarg",
					"unknown conversion type %s",type_c);
				break;
		   }
		break;
	   }
        UFREE (type_c);
	return(found);
boolean:

	/*
	 *  if list is NULL then return NOW; the following "for" loop
	 *  the pointer ARGLIST + (NLIST-1) gives ARGLIST -1 which
	 *  does not test to < ARGHEAD for some reason
         */

	if(NLIST <= 0) {
          UFREE (type_c);
          return(found);
        }

	sprintf(noname,"no%s",name);
	hno = ga_compute_hash(noname);
	hyes= ga_compute_hash(  name);
	found=0;
	/* search list backwards, stopping at first find */
	for(alptr= ARGLIST +(NLIST-1); alptr >= ARGLIST; alptr--)
	   {
		if(alptr->hash != hno && alptr->hash != hyes) continue;
		if(strcmp(alptr->argname,  name)== 0)
		   {
			if(alptr->argval[0] == '\0') *ival= 1;
			 else *ival= atol(alptr->argval);
			found++;
			break;
		   }
		if(strcmp(alptr->argname,noname)== 0)
		   {	*ival= 0; found++; break; }
	   }
        UFREE (type_c);
	return(found);
}

static int
ga_compute_hash(const char *s)
{
	int h;
	h= s[0];
	if(s[1]) h |= (s[1])<<8;	else return(h);
	if(s[2]) h |= (s[2])<<16;	else return(h);
	if(s[3]) h |= (s[3])<<24;
	return(h);
}

static int
ga_getvector (const char *list, const char *type, char *val)
{
	const char *p;
	int index, cnt;
	const char *valptr;
        char **strptr, *svalptr, sval[MAXVALUE], *ptr;
	int limit;
	int ival, *iptr;
	float fval, *fptr;
	double dval, *dptr;

	limit = MAXVECTOR;
	if(type[2] == '(' || type[2] == '[') limit= atol(&type[3]);
	if(limit <= 0)
		return ga_getarg_err("getarg","bad limit=%d specified",limit);
	/*fprintf(stderr,"limit=%d\n",limit);*/
	index= 0;
	p= list;
	while(*p != '\0'  && index < limit)
	   {
		cnt=1;
	 backup: /* return to here if we find a repetition factor */
		while(*p == ' ' || *p == '\t') p++;
		if(*p == '\0') return(index);
		valptr= p;
	getvalue: /* return here if valid value in char*[] arg */
		while( *p != ',' && *p != '*' && *p != 'x' && *p != 'X' &&
			*p != '\0') p++;
		if (type[1] == 's' && ((*p == ',' && p[-1] == '\\')
				       || *p == '*' || *p == 'x' || *p == 'X'))
		{	p++; goto getvalue; }
		if((*p == '*' || *p == 'x' || *p == 'X') &&
		   type[1] != 's')
		   {
			cnt= atol(valptr);
			if(cnt <= 0)
				return ga_getarg_err("getarg",
					"bad repetition factor=%d specified",
					 cnt);
			if(index+cnt > limit) cnt= limit - index;
			p++;
			goto backup;
		   }
		/*fprintf(stderr,"index=%d cnt=%d p=%s$\n",index,cnt,p);*/
		switch(type[1])
		   {
			case 'd':
				iptr= (int *) val;
				ival= (int) strtol(valptr, &ptr, 0);
				if (ptr == valptr)
					index = 0;
				else
				{
					if (iptr+index == NULL)
						return ga_getarg_err("getarg",
			       		          "NULL vector ptr at index %d.", index);
					while(cnt--)
					       	iptr[index++] = ival;
				}
				break;
			case 'f':
				fptr= (float *) val;
				fval= (float) strtod(valptr, &ptr);
				if (ptr == valptr)
					index = 0;
				else
				{
					if (fptr+index == NULL)
						return ga_getarg_err("getarg",
				       	          "NULL vector ptr at index %d", index);
					while(cnt--)
						fptr[index++] = fval;
				}
				break;
			case 'F':
				dptr= (double *) val;
				dval= strtod(valptr, &ptr);
				if (ptr == valptr)
					index = 0;
				else
				{
					if (dptr+index == NULL)
						return ga_getarg_err("getarg",
				       	          "NULL vector ptr at index %d", index);
					while(cnt--)
						dptr[index++] = dval;
				}
				break;
			case 's':
				svalptr = sval;
				strptr = (char **) val;
                                /* ensure null terminator for strcpy below */
				*svalptr = '\0';
				while (*valptr != '\'' && *valptr != '\0')
				{
					if (*valptr == '\\')
						valptr++;
				       	*svalptr++ = *valptr++;
				}
                                /* ensure null terminator for strcpy below */
                                *svalptr= '\0';
				if (strptr+index == NULL)
					return ga_getarg_err("getarg",
		       	       		  "NULL vector ptr at index %d", index);
                                /* can't use strncpy here as size of
                                 * destination buffer is unknown; remember
                                 * that strncpy pads the entire size of buffer
                                 * with null bytes, hence causing memory
                                 * corruption if actual size of destination is
                                 * less than what is specified
                                 */
                                /* but let's do a bit sanity checking/fixing
                                 * just in case */
                                if (strlen(sval) > MAXVALUE) {
                                  sval[MAXVALUE-1] = '\0';
                                }
				strcpy(strptr[index++], sval);
				if (*p != '\0') p++;
				break;
			default:
				return ga_getarg_err("getarg",
					"bad vector type=%c specified",type[1]);
				break;
		   }
		/* if conversion couldn't be made return 0 */
		if (index == 0)
			break;
		if (*p != '\0')
			p++;
	   }
	return(index);
}

#ifdef HAVE_STDARG_H
static int
ga_getarg_err (const char *subname, const char *format, ...)
#else
static int
ga_getarg_err (va_alist) va_dcl
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
	if(format == NULL) return GETARG_ERROR;

	fprintf(stderr,"\n***** ERROR in %s[%s] *****\n\t",
		(PROGNAME == NULL ? "(unknown)" : PROGNAME),subname);
	vfprintf(stderr, format, va);
	fprintf(stderr,"\n");
	return GETARG_ERROR;
}

char *
getsarg (const char *parname, const char *defvalue)
{
	char parvalue[MAXVALUE];

	parvalue[0] = '\0';

	if(defvalue)
	{
		strncpy(parvalue, defvalue, MAXVALUE);
		parvalue[MAXVALUE-1] = '\0';
	}

	getarg(parname, "s", parvalue);

	if(! defvalue && parvalue[0] == '\0') return( (char *) 0 );

	else return(strdup(parvalue));
}
