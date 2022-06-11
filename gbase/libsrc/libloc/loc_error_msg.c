
/*
 * Copyright (c) 1995-1996 Science Applications International Corporation.
 *

 * NAME
 *	loc_error_msg -- Print specific location failure condition message

 * FILE
 *	loc_error_msg.c

 * SYNOPSIS
 *	char *
 *	loc_error_msg (error_code)
 *	int	error_code;	(i) Event location error code

 * DESCRIPTION
 *	Function.  Print out specific location failure condition message
 *	given input error code.  The error condition character string 
 *	message, as obtained from *locator_error_table[], is returned.

 * DIAGNOSTICS
 *	If input error code is out-of-range, then the message,
 *	"Locator: Input error code is out-of-range!", will be returned.

 * NOTES
 *	None.

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy,  7/26/95,	Created.
 */


#include "config.h"
#include <stdio.h>
#include "libloc.h"
#include "loc_defs.h"

char
*locator_error_table[] =
{
/* 0 */ "Locator: Successful convergence achieved!",
/* 1 */ "Locator: Maximum number of iterations exceeded!",
/* 2 */ "Locator: Divergent solution encountered!",
/* 3 */ "Locator: Insufficient defining data before location is even attempted!",
/* 4 */ "Locator: Insufficient defining data remains due to hole in T-T table(s)!",
/* 5 */ "Locator: Insufficient defining data remains due to T-T table extrapolation!",
/* 6 */ "Locator: SVD routine cannot decompose given matrix!",
/* 7 */ "Locator: Condition number of solution is too great to continue!",
/* 8 */ "Locator: Lat/Lon value(s) out of range given a fixed lat/lon!",
/* 9 */ "Locator: Warning - No observations to process!",
/*10 */ "Locator: Bad input assoc data!",
/*11 */ "Locator: Bad input origin data!",
/*12 */ "Locator: Bad input origerr data!",
/*13 */ "Locator: Mismatch between input arrival/assoc data!",
/*14 */ "Locator: Insufficient defining data remains due to large residual restriction!",
/*15 */ "Locator: Insufficient defining data remains due to TT correction restriction!",
/*16 */ "Locator: Insufficient defining data remains while attempting to locate event!",
/*17 */ "TT Tables: Null phase id list!",
/*18 */ "TT Tables: Error opening tables!",
/*19 */ "TT Tables: Error reading tables, unexpected EOF!",
/*20 */ "TT Tables: Memory allocation error while trying to read T-T tables!",
/*21 */ "TT Tables: Could not open velocity model specification file!",
/*22 */ "Site Table: Null station list!",
/*23 */ "Locator: Error reading SSSC tables!",
/*24 */ "Locator: Error reading SRST tables!",
/*25 */ "Locator: Error reading test-site correction tables!",
/*26 */ "Locator: Requested test-site region not available!",
/*27 */ "Locator: Error encountered while trying to read ellipticity corr. tables!",
/*28 */ "LP Tables: Error encountered while trying to read LP T-T tables!",
/*29 */ "TT Tables: Cannot set Sta_Pt structure!  site table likely missing!"
};

char
input_code_out_of_range[] =
{
	"Locator: Input error code is out-of-range!"
};

static	int	max_elements = ((sizeof locator_error_table)/sizeof(char*));


char *
loc_error_msg (int error_code)
{

	if (error_code < 0 || error_code >= max_elements)
	    return (input_code_out_of_range);
	else
	    return (locator_error_table[error_code]);
}


