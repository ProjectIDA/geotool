
/*
 * Copyright (c) 1995-1997 Science Applications International Corporation.
 *

 * NAME
 *	last_leg -- What is phase type of last leg (i.e., P or S) ?

 * FILE
 *	last_leg.c

 * SYNOPSIS
 *	int
 *	last_leg (phase)
 *	char	*phase;		(i) Seismic phase

 * DESCRIPTION
 *	Function.  This routine determines whether the last leg of a given
 *	input phase is a P or S.  This is done to properly handle generic
 *	elevation corrections in the travel-time handling facilities.

 * DIAGNOSTICS
 *	If input phase cannot be identified as having its final leg being
 *	P or S, then the approiate code will be returned (as defined by
 *	BOGUS_PHASE_TYPE or JUST_IGNORE_THIS_PHASE).

 * FILES
 *	None.

 * NOTES
 *	None.

 * SEE ALSO
 *	Travel-time handling facilities in file, trv_time_specific.c.

 * AUTHOR
 *	Walter Nagy, 10/95,	Created.
 */


#include "config.h"
#include <stdio.h>
#include "locp.h"

int
last_leg (char *phase)
{
	int	i;


	if (! phase)
	{
	    return (BOGUS_PHASE_TYPE);
	}

	i = strlen (phase) - 1;

	/*
	 * Identify given input phase as a potential back-branch.  If it
	 * is, then decrement index counter (i) by 2 to compensate.
	 */

	if (i > 2 && (phase[i-1] == '_'))
	    i -= 2;

	if (phase[i] == 'P')
	{
	    /* Last leg is a teleseismic P */
	    return (LAST_LEG_IS_P);
	}

	else if (phase[i] == 'S')
	{
	    /* Last leg is a teleseismic S */
	    return (LAST_LEG_IS_S);
	}

	else if (phase[i] == 'f')
	{
	    if (phase[i-1] == 'd')
	    {
		if (phase[i-2] == '2' || phase[i-2] == '3')
		    --i;
		if (phase[i-2] == 'P')
		{
		    /* Last leg is a teleseismic P (df branch) */
		    return (LAST_LEG_IS_P);
		}
		else if (phase[i-2] == 'S')
		{
		    /* Last leg is a teleseismic S (df branch) */
		    return (LAST_LEG_IS_S);
		}
		else
		    return (BOGUS_PHASE_TYPE);
	    }
	    else if (STREQ (phase, "Pdiff") || STREQ (phase, "Pdif") ||
		     STREQ (phase+1, "Pdiff") || STREQ (phase+1, "Pdif"))
	    {
		/* Last leg is a teleseismic P */
		return (LAST_LEG_IS_P);
	    }
	    else if (STREQ (phase, "Sdiff") || STREQ (phase, "Sdif") ||
		     STREQ (phase+1, "Sdiff") || STREQ (phase+1, "Sdif"))
	    {
		    /* Last leg is a teleseismic S */
		return (LAST_LEG_IS_S);
	    }
	    else
		return (BOGUS_PHASE_TYPE);
	}

	else if (phase[i] == 'b')
	{
	    if (phase[i-1] == 'a')
	    {
		if (phase[i-2] == '2' || phase[i-2] == '3')
		    --i;
		if (phase[i-2] == 'P')
		{
		    /* Last leg is a teleseismic P (ab branch) */
		    return (LAST_LEG_IS_P);
		}
		else if (phase[i-2] == 'S')
		{
		    /* Last leg is a teleseismic S (ab branch) */
		    return (LAST_LEG_IS_S);
		}
		else
		    return (BOGUS_PHASE_TYPE);
	    }
	    else if (STREQ (phase, "Pb"))
	    {
		/* Last leg is a regional Pb branch */
		return (LAST_LEG_IS_P);
	    }
	    else if (STREQ (phase, "Sb"))
	    {
		/* Last leg is a regional Sb branch */
		return (LAST_LEG_IS_S);
	    }
	    else
		return (BOGUS_PHASE_TYPE);
	}

	else if (phase[i] == 'c')
	{
	    if (phase[i-1] == 'b')
	    {
		if (phase[i-2] == '2' || phase[i-2] == '3')
		    --i;
		if (phase[i-2] == 'P')
		{
		    /* Last leg is a teleseismic P (bc branch) */
		    return (LAST_LEG_IS_P);
		}
		if (phase[i-2] == 'S')
		{
		    /* Last leg is a teleseismic S (bc branch) */
		    return (LAST_LEG_IS_S);
		}
		else
		    return (BOGUS_PHASE_TYPE);
	    }
	    else if (phase[i-1] == 'a')
	    {
		if (phase[i-2] == '2' || phase[i-2] == '3')
		    --i;
		if (phase[i-2] == 'S')
		{
		    /* Last leg is a teleseismic S (ac branch) */
		    return (LAST_LEG_IS_S);
		}
		else
		    return (BOGUS_PHASE_TYPE);
	    }
	    else
		return (BOGUS_PHASE_TYPE);
	}

	else if (phase[i] == '2' || phase[i] == '3')
	{
	    if (phase[i-1] == 'P')
	    {
		/* Last leg is a teleseismic P (df branch) */
		return (LAST_LEG_IS_P);
	    }
	    else if (phase[i-1] == 'S')
	    {
		/* Last leg is a teleseismic S (df branch) */
		return (LAST_LEG_IS_S);
	    }
	    else
		return (BOGUS_PHASE_TYPE);
	}

	else if (phase[i] == 'n')
	{
	    if (phase[i-1] == 'P')
	    {
		/* Last leg is a regional Pn branch */
		return (LAST_LEG_IS_P);
	    }
	    else if (phase[i-1] == 'S')
	    {
		/* Last leg is a regional Sn branch */
		return (LAST_LEG_IS_S);
	    }
	    else
		return (BOGUS_PHASE_TYPE);
	}

	else if (phase[i] == 'g')
	{
	    if (phase[i-1] == 'P')
	    {
		/* Last leg is a regional Pg branch */
		return (LAST_LEG_IS_P);
	    }
	    else if (phase[i-1] == 'L' || phase[i-1] == 'S')
	    {
		/* Last leg is a regional S branch (Lg, Sg or Rg) */
		return (LAST_LEG_IS_S);
	    }
	    else if (phase[i-1] == 'R')
	    {
		/* Ignore Rg */
		return (JUST_IGNORE_THIS_PHASE);
	    }
	    else
		return (BOGUS_PHASE_TYPE);
	}

	else if (STREQ (phase, "LR") || STREQ (phase, "LQ"))
	{
	    /* Ignore Rayleigh and Love waves */
	    return (JUST_IGNORE_THIS_PHASE);
	}

	else if (phase[i] == 'x' || phase[0] == 'T' || phase[0] == 'H')
	{
	    /* Ignore Px, Sx, Tx, T and H phase types */
	    return (JUST_IGNORE_THIS_PHASE);
	}

	else if (phase[0] == 'I')
	{
	    /* Ignore infrasound phase types */
	    return (JUST_IGNORE_THIS_PHASE);
	}

	else if (phase[0] == 'n' || phase[0] == 'N')
	{
	    /* Ignore noise phase types */
	    return (JUST_IGNORE_THIS_PHASE);
	}

	else
	{
	    return (BOGUS_PHASE_TYPE);
	}
}


