
/*
 * Copyright (c) 1997 Science Applications International Corporation.
 *

 * NAME
 *	mag_error_msg -- Print specific magnitude failure condition message

 * FILE
 *	mag_error_msg.c

 * SYNOPSIS
 *	char *
 *	mag_error_msg (error_code)
 *	int	error_code;	(i) Event location error code

 * DESCRIPTION
 *	Function.  Print out specific magnitude failure condition message
 *	given input error code.  The error condition character string 
 *	message, as obtained from *mag_error_table[], is returned.

 * DIAGNOSTICS
 *	If input error code is out-of-range, then the message,
 *	"Magnitude: Input error code is out-of-range!", will be returned.

 * NOTES
 *	Error codes are broken down into 4 distinct areas, prefixed as:
 *	    MDreadErr:	Problem encountered while reading mag_descrip info
 *	    SSgetErr:	Problem encountered while attempting to set sta_pt info
 *	    StaMagErr:	Failure during station magnitude processing
 *	    NetMagErr:	Failure during network magnitude processing

 * SEE ALSO
 *	None.

 * AUTHOR
 *	Walter Nagy,  9/2/97,	Created.
 */


#include <stdio.h>
#include "libmagnitude.h"


char
*mag_error_table[] =
{
/* 0 */ "Magnitude: Successful magnitude computed!",
/* 1 */ "MDreadErr1: Cannot open MDF!",
/* 2 */ "MDreadErr2: MDF incorrectly formatted!",
/* 3 */ "MDreadErr3: No matching TLtype found for info specified in TLSF!",
/* 4 */ "MDreadErr4: Error allocating memory while reading mag info!",
/* 5 */ "SSgetErr1: No input site table info available for Sta_Pt!",
/* 6 */ "SSgetErr2: Error allocating memory while trying to set Sta_Pt info!",
/* 7 */ "NetMagErrX: Cannot set Sta_Pt structure!  Site table likely missing!"
};

char
input_magcode_out_of_range[] =
{
	"Magnitude: Input error code is out-of-range!"
};

static	int	max_elements = ((sizeof mag_error_table)/sizeof(char*));


char *
#ifdef __STDC__
mag_error_msg (int error_code)
#else
mag_error_msg (error_code)
int	error_code;
#endif
{

	if (error_code < 0 || error_code >= max_elements)
	    return (input_magcode_out_of_range);
	else
	    return (mag_error_table[error_code]);
}


