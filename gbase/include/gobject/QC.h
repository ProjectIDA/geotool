#ifndef _QC_H_
#define _QC_H_

#include "gobject/GMethod.h"
#include "libdataqc.h"

/** 
 *  A GMethod used to record a cut waveform operation.
 *
 *  @see new_GMethodPart
 *  @see new_GObjectPart
 *
 * </pre>
 */ /*<pre>*/

typedef struct
{
	GObjectPart		core;
	GMethodPart		gm;

	/* QC members */
	QCDef	qcdef;
} _QC, *QC;

QC new_QC(QCDef *qcdef);
void QC_addMethod(void);
GMethod QC_methodCreate(TimeSeries ts, const char *s);

#endif
