/*	SccsId:	%W%	%G%	*/

#ifndef _CSSIO_H_
#define _CSSIO_H_

#include "gobject/GObject.h"
#include "gobject/cssObjects.h"
#include "css/wfdisc.h"

/**
 *  A GObject that holds information about wfdisc I/O.
 *
 *  @see new_WfdiscIO
 *  @see WFDISC30
 *  @see new_GObjectPart
 */

/** 
 *  A GObject that holds information about wfdisc I/O.
 *  @member db Database account/password.
 *  @member dir Directory containing the wfdiscs.
 *  @member prefix Prefix for .wfdisc,.arrival, files.
 *  @member wfdisc_File wfdisc filename (complete path).
 *  @member wfdisc_index Index of wfdisc record in .wfdisc.
 *  @member pts_needed Read function argument.
 *  @member wf A wfdisc structure.
 */
typedef struct
{
        GObjectPart	core;

	int		db;		/* database account/password */
	int		dir;		/* directory containing the wfdiscs */
	int		prefix;		/* prefix for .wfdisc,.arrival,files */
	int		wfdisc_file;	/* wfdisc filename (complete path) */
	int		wfdisc_index;	/* index of wfdisc record in .wfdisc */
	int		pts_needed;
	WFDISC30 	wf;		/* wfdisc */
} _WfdiscIO, *WfdiscIO;

typedef struct
{
	GObjectPart	core;

	WFDISC30	wf;
	CssOrigin	o;
} wavIO, *WavIO;


WfdiscIO new_WfdiscIO(void);
WavIO new_WavIO(void);



#endif /* _CSSIO_H_ */
