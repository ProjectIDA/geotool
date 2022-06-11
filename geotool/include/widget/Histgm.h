#ifndef	HISTGM_H_
#define	HISTGM_H_

#include "widget/Axes.h"

#define XtNdataColor 			(char *)"dataColor"
#define XtNselectCallback		(char *)"selectCallback"
#define XtNstretchCallback		(char *)"stretchCallback"

#define XtCDataColor 			(char *)"DataColor"
#define XtCSelectCallback		(char *)"SelectCallback"
#define XtCStretchCallback		(char *)"StretchCallback"

#ifndef XtRDouble
#define XtRDouble	(char *)"double"
#endif

/// @cond
typedef struct _HistgmClassRec	*HistgmWidgetClass;
typedef struct _HistgmRec	*HistgmWidget;

extern WidgetClass		histgmWidgetClass;
/// @endcond

#ifndef HANN_TAPER
#define HANN_TAPER 0
#endif

void HistgmInput(HistgmWidget w, int num_colors, Pixel *colors,
	int num_stipples, Pixmap *stipples, double *lines, int num_bins,
	float *bins, float data_min, float data_max);
void HistgmInputData(HistgmWidget w, int npts, float *z, float no_data_flag,
	int num_bins, int num_colors, Pixel *colors, int num_stipples,
	Pixmap *stipples, double *lines);
void HistgmLines(HistgmWidget w, int num_colors, int num_stipples,
			double *lines);
void HistgmColors(HistgmWidget w, int num_colors, Pixel *pixels);
Widget HistgmCreate(Widget parent, String name, ArgList arglist,
			Cardinal argcount);

#endif	/* HISTGM_H_ */
