#ifndef _GMETHOD_H_
#define _GMETHOD_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <sys/param.h>

#include "gobject/TimeSeries.h"
#include "libgnet.h"

/**
 *  A GMethod is a GObject used to hold information about a processing
 *  method for TimeSeries objects. The function protoype is
 *  GMethod_applyMethod. A second function prototype, GMethod_toStringMethod,
 *  is available to produce a character string method descriptor.
 *
 *  @see GObject
 *  @see TimeSeries
 *
 */

/** 
 *  GMethodPart structure. This is an opaque structure pointer here to protect
 *  its contents, which should not be changed after creation by
 *  new_GMethodPart().
 *  @see new_GMethodPart
 */
typedef struct GMethod_struct	*GMethodPart;

/** 
 *  GMethod structure. A GMethod is any structure that has a GObjectPart as
 *  its first member and a GMethodPart as its second member.
 */
typedef struct
{
	GObjectPart	core;
	GMethodPart	gm;
} _GMethod, *GMethod;


/** 
 *  Apply a processing method to TimeSeries objects.
 *
 *  @param gm	A GMethod object pointer.
 *  @param n  	The number of TimeSeries objects.
 *  @param ts 	The TimeSeries objects.
 */
typedef bool (*GMethod_applyMethod)(GMethod gm, int n, TimeSeries *ts,
			const char **err_msg);

/** 
 *  Create a string method descriptor.
 *
 *  @param gm A GMethod object pointer.
 */
typedef const char *(*GMethod_toStringMethod)(GMethod gm);

typedef GMethod (*GMethodCreate)(TimeSeries ts, const char *args);

GMethodPart new_GMethodPart(int method_type, GMethod_applyMethod apply,
			GMethod_toStringMethod toString);
const char *GMethod_toString(GMethod method);
void GMethod_free(GMethod o);

typedef void (*WriteMethod)(GMethod gm, FILE *fp);
typedef GMethod (*ReadMethod)(FILE *fp);

bool GMethod_addMethod(const char *method_name, WriteMethod writeMethod,
			ReadMethod readMethod);

bool GMethod_apply(GMethod method, int num_waveforms, CPlotData **cd_list);
bool GMethod_applyMethods(int num_methods, GMethod *method,
                        int num_waveforms, CPlotData **cd_list);
bool GMethod_change(GMethod method, int num_waveforms, CPlotData **cd_list);
bool GMethod_changeMethods(int num_methods, GMethod *method,
                        int num_waveforms, CPlotData **cd_list);
bool GMethod_remove(int object_type, int num_waveforms, CPlotData **cd_list);
bool GMethod_removeMethods(int num_methods, int *object_type,
                        int num_waveforms, CPlotData **cd_list);
bool GMethod_update(CPlotData *cd);
bool GMethod_changeTs(int num_methods, GMethod *method, int num_waveforms,
                        TimeSeries *ts);
GMethod GMethod_getMethod(TimeSeries ts, const char *method_name);
bool GMethod_applyTs(GMethod method, TimeSeries ts);

/* ******* reread.c ********/
void getFormatStructs(int *num_formats, FormatStruct **formats);
bool RereadTimeSeries(TimeSeries ts, char *sta, char *chan);


typedef struct
{
        char            *name;          /* ie IIR, Taper, Rotate, etc. */
        WriteMethod     writeMethod;
        ReadMethod      readMethod;
        int             num_instances;
} Method;

int GMethod_registeredMethods(Method **methods);

#define SINGLE_TS	1
#define ARRAY_TS	2

#endif
