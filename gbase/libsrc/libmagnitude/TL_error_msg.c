
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * NAME
 *	TL_error_msg -- Print specific TL failure condition message

 * FILE
 *	TL_error_msg.c

 * SYNOPSIS
 *	char *
 *	TL_error_msg (error_code)
 *	int	error_code;	(i) TL error code

 * DESCRIPTION
 *	Function.  Print out specific transmission loss (TL) failure 
 *	condition message given input error code.  The error condition 
 *	character string message, as obtained from *TL_error_table[], is 
 *	returned.

 * DIAGNOSTICS
 *	If input error code is out-of-range, then the message,
 *	"TL: Input error code is out-of-range!", will be returned.

 * NOTES
 *	Error codes are broken down into 3 distinct areas, prefixed as:
 *	    TLreadErr:	Problem encountered while reading TL info
 *	    TLreadWarn:	Warning condition encountered while reading TL info
 *	    TLgetErr:	Failure encountered during access to TL info

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy,  9/27/97,	Created.
 */


#include <stdio.h>
#include "libmagnitude.h"
#include "tl_defs.h"


char
*TL_error_table[] =
{
/* 0 */ "TL: Successful TL condition!",
/* 1 */ "TLreadWarn1: A requested TL file was not found!",
/* 2 */ "TLreadErr1: Cannot open TLSF!",
/* 3 */ "TLreadErr2: TLSF incorrectly formatted!",
/* 4 */ "TLreadErr3: No TL tables could be found!",
/* 5 */ "TLreadErr4: TL table incorrectly formatted!",
/* 6 */ "TLreadErr5: TL modelling error table incorrectly formatted!",
/* 7 */ "TLreadErr6: TL test-site corr. file incorrectly formatted!",
/* 8 */ "TLreadErr7: Error allocating memory while reading TL info!"
};

char
input_TLcode_out_of_range[] =
{
	"TL: Input error code is out-of-range!"
};

static	int	max_elements = ((sizeof TL_error_table)/sizeof(char*));


char *
#ifdef __STDC__
TL_error_msg (int error_code)
#else
TL_error_msg (error_code)
int	error_code;
#endif
{

	if (error_code < 0 || error_code >= max_elements)
	    return (input_TLcode_out_of_range);
	else
	    return (TL_error_table[error_code]);
}


