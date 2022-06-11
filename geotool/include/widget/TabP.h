#ifndef _TABP_H_
#define _TABP_H_

#include <Xm/Xm.h>
#include <Xm/ManagerP.h>

#include "widget/Tab.h"

/// @cond

typedef struct
{
	char *label;
	Pixel fg;
} TabColor;

typedef struct
{
	Widget		label;	/* top title */
	Widget		arrowLeft;
	Widget		arrowRight;
	Boolean		ignore_change_managed;
	Boolean		use_tab_color;
	Boolean		use_top_tab_color;
	char		*title;
	int		num_tab_colors;
	TabColor	*tab_colors;
	char		**tab_labels;

	Dimension	shadow_thickness;
	int		onTop;
	int		*tab_widths;
	int		*label_y;
	int		tab_height;
	int		margin;
	int		shift;
	int		max_shift;

	XFontStruct	*font;
	Pixel		tab_fg;
	Pixel		top_tab_fg;
	GC		gc;
	XtAppContext	app_context;

	XtCallbackList	tab_callbacks;
	XtCallbackList	tab_menu_callbacks;
	XtCallbackList	insensitive_tab_callbacks;

} TabPart;

typedef struct
{
	int	empty;
} TabClassPart;

typedef struct _TabRec
{
	CorePart	core;
	CompositePart	composite;
	ConstraintPart	constraint;
	XmManagerPart	manager;
	TabPart	tab;
} TabRec;

typedef struct _TabClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	TabClassPart		tab_class;
} TabClassRec;

extern TabClassRec	tabClassRec;

/// @endcond

#endif
