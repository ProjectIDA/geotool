/** \file Dial.c
 *  \brief Defines widget Dial.
 *  \author Douglas A. Young
 */
#include "config.h"
/*
 *                Copyright 1989 by Douglas A. Young
 *
 *                         All Rights Reserved
 *
 * Permission to use, copy, modify, and distribute this software
 * for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation. The author disclaims all warranties with 
 * regard to this software, including all implied warranties of 
 * merchantability and fitness.
 *
 * Comments and additions may be sent the author at:
 *
 *  dayoung@hplabs.hp.com
 */

/********************************************************
 * Dial.c: The Dial Widget Methods
 *********************************************************/
#include <stdio.h>
#include <math.h>
#include <X11/IntrinsicP.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/CoreP.h>
#include "widget/DialP.h"
#include "widget/Dial.h"

#define  RADIANS(x)  (M_PI * 2.0 * (x) / 360.0)
#define  DEGREES(x)  ((x) / (M_PI * 2.0) * 360.0)
#define  MIN_ANGLE   0.0
#define  MAX_ANGLE   360.0
#define  ABSVAL(a)   (( (a) >= 0 ) ? (a) : (-a))

static void select_dial(XsDialWidget w, XEvent *event);
static void Initialize(Widget w_req, Widget w_new);
static void Redisplay(Widget w, XEvent *event, Region region);
static void Resize(Widget w);
static void Destroy(Widget w);
static void calculate_indicator_pos(XsDialWidget w);
static void calculate_arrow1_pos(XsDialWidget w);
static void calculate_arrow2_pos(XsDialWidget w);
static Boolean SetValues(XsDialWidget current, XsDialWidget request,
			XsDialWidget new_index);

static char defaultTranslations[] =  
"<Btn1Down>: select()\n\
 <Btn1Motion>: select()\n\
 <Btn1Up>: select()";

static XtActionsRec actionsList[] = {
  { "select",   (XtActionProc) select_dial},
};

static XtResource resources[] = {
  {XtNmarkers, XtCMarkers, XtRInt, sizeof (int),
    XtOffset(XsDialWidget, dial.markers), XtRString, (XtPointer)"12" },
  {XtNminimum, XtCMin, XtRInt, sizeof (int),
    XtOffset(XsDialWidget, dial.minimum), XtRString,(XtPointer) "0"  },
  {XtNmaximum, XtCMax, XtRInt, sizeof (int),
    XtOffset(XsDialWidget, dial.maximum), XtRString,(XtPointer) "360"},
  {XtNindicatorColor, XtCColor, XtRPixel, sizeof (Pixel),
    XtOffset(XsDialWidget, dial.indicator_color), 
    XtRString,(XtPointer) "Black"                                    },
{XtNposition, XtCPosition, XtRPosition, sizeof (Position),
   XtOffset(XsDialWidget, dial.position), XtRString,(XtPointer) "0"  },
  {XtNmarkerLength, XtCLength, XtRDimension, sizeof (Dimension),
    XtOffset(XsDialWidget, dial.marker_length),
    XtRString, (XtPointer)"5"                                        },
  {XtNforeground, XtCForeground, XtRPixel, sizeof (Pixel),
    XtOffset(XsDialWidget, dial.foreground), 
    XtRString, (XtPointer)"Black"                                    },
  {XtNselect, XtCCallback, XtRCallback, sizeof(XtPointer),
    XtOffset (XsDialWidget, dial.select), 
    XtRCallback, (XtPointer)NULL                                     },
  {XtNarrow1Visible, XtCArrow1Visible, XtRBoolean, sizeof(Boolean),
    XtOffset (XsDialWidget, dial.arrow1Visible), XtRString, (XtPointer)"False"},
  {XtNarrow1Color, XtCArrow1Color, XtRPixel, sizeof(Pixel),
    XtOffset (XsDialWidget, dial.arrow1Color), XtRString, (XtPointer)"Black"},
  {XtNarrow1Position, XtCArrow1Position, XtRPosition, sizeof(Position),
    XtOffset (XsDialWidget, dial.arrow1Position), XtRString,(XtPointer) "0"},
  {XtNarrow2Visible, XtCArrow2Visible, XtRBoolean, sizeof(Boolean),
    XtOffset (XsDialWidget, dial.arrow2Visible), XtRString,(XtPointer) "False"},
  {XtNarrow2Color, XtCArrow2Color, XtRPixel, sizeof(Pixel),
    XtOffset (XsDialWidget, dial.arrow2Color), XtRString,(XtPointer) "Black"},
  {XtNarrow2Position, XtCArrow2Position, XtRPosition, sizeof(Position),
    XtOffset (XsDialWidget, dial.arrow2Position), XtRString,(XtPointer) "0"}
 };

XsDialClassRec  XsdialClassRec = {
     /* CoreClassPart */
  {
   (WidgetClass) &widgetClassRec,  /* superclass            */
   "Dial",                         /* class_name            */
   sizeof(XsDialRec),              /* widget_size           */
   NULL,                           /* class_initialize      */
   NULL,                           /* class_part_initialize */
   FALSE,                          /* class_inited          */
   (XtInitProc)Initialize, 	   /* initialize            */
   NULL,                           /* initialize_hook       */
   XtInheritRealize,               /* realize               */
   actionsList,                    /* actions               */
   XtNumber(actionsList),          /* num_actions           */
   resources,                      /* resources             */
   XtNumber(resources),            /* num_resources         */
   NULLQUARK,                      /* xrm_class             */
   TRUE,                           /* compress_motion       */
   TRUE,                           /* compress_exposure     */
   TRUE,                           /* compress_enterleave   */
   TRUE,                           /* visible_interest      */
   Destroy,                        /* destroy               */
   Resize,                         /* resize                */
   Redisplay,                      /* expose                */
   (XtSetValuesFunc)SetValues,	   /* set_values            */
   NULL,                           /* set_values_hook       */
   XtInheritSetValuesAlmost,       /* set_values_almost     */
   NULL,                           /* get_values_hook       */
   NULL,                           /* accept_focus          */
   XtVersion,                      /* version               */
   NULL,                           /* callback private      */
   defaultTranslations,            /* tm_table              */
   NULL,                           /* query_geometry        */
   NULL,                           /* display_accelerator   */
   NULL,                            /* extension             */
   },
      /* Dial class fields */
  {
   0,                              /* ignore                */
   }
};

WidgetClass XsdialWidgetClass = (WidgetClass) &XsdialClassRec;

static void
Initialize(Widget w_req, Widget w_new)
{
  XsDialWidget request = (XsDialWidget)w_req;
  XsDialWidget new_index = (XsDialWidget)w_new;
  XGCValues values;
  XtGCMask  valueMask;
  /*
   * Make sure the window size is not zero. The Core 
   * Initialize() method doesn't do this.
   */
  if (request->core.width == 0)
    new_index->core.width = 100;
  if (request->core.height == 0)
    new_index->core.height = 100;
  /*
   * Make sure the min and max dial settings are valid.
   */
  if (new_index->dial.minimum >= new_index->dial.maximum) {
    XtWarning ("Maximum must be greater than the Minimum");
    new_index->dial.minimum = 0;
    new_index->dial.maximum = 360;
  }
  if (new_index->dial.position > new_index->dial.maximum) {
    XtWarning ("Position exceeds the Dial Maximum");
    new_index->dial.position =  new_index->dial.maximum;
  }
  if (new_index->dial.position < new_index->dial.minimum) {
    XtWarning ("Position is less than the Minimum");
    new_index->dial.position =  new_index->dial.minimum;
  }
  /*
   * Allow only MAXSEGMENTS / 2 markers
   */
  if(new_index->dial.markers > MAXSEGMENTS / 2){
    XtWarning ("Too many markers");
    new_index->dial.markers = MAXSEGMENTS / 2;
  }
  /*
   * Create the graphics contexts used for the dial face 
   * and the indicator.
   */
  valueMask = GCForeground | GCBackground;
  values.foreground = new_index->dial.foreground;
  values.background = new_index->core.background_pixel;
  new_index->dial.dial_GC = XtGetGC ((Widget)new_index, valueMask, &values);  

  values.foreground = new_index->dial.indicator_color;
  new_index->dial.indicator_GC = XtGetGC ((Widget)new_index, valueMask, &values);  

  values.foreground = new_index->dial.arrow1Color;
  new_index->dial.arrow1_GC = XtGetGC ((Widget)new_index, valueMask, &values);  

  values.foreground = new_index->dial.arrow2Color;
  new_index->dial.arrow2_GC = XtGetGC ((Widget)new_index, valueMask, &values);  

  valueMask = GCForeground | GCBackground;
  values.foreground = new_index->core.background_pixel;
  values.background = new_index->dial.indicator_color;
  new_index->dial.inverse_GC = XtGetGC ((Widget)new_index, valueMask, &values);   

  values.background = new_index->dial.arrow1Color;
  new_index->dial.inverse_arrow1_GC = XtGetGC ((Widget)new_index, valueMask, &values);   

  values.background = new_index->dial.arrow2Color;
  new_index->dial.inverse_arrow2_GC = XtGetGC ((Widget)new_index, valueMask, &values);   

  Resize((Widget)new_index);
}

static void
Destroy(Widget widget)
{
  XsDialWidget w = (XsDialWidget)widget;

  XtDestroyGC (w->dial.indicator_GC);
  XtDestroyGC (w->dial.inverse_GC);
  XtDestroyGC (w->dial.inverse_arrow1_GC);
  XtDestroyGC (w->dial.inverse_arrow2_GC);
  XtDestroyGC (w->dial.dial_GC);
  XtDestroyGC (w->dial.arrow1_GC);
  XtDestroyGC (w->dial.arrow2_GC);
  XtRemoveAllCallbacks ((Widget)w, XtNselect);
}

static void
Resize(Widget widget)
{
  XsDialWidget w = (XsDialWidget)widget;
  double    angle, cosine, sine, increment;
  int       i; 
  XPoint   *ptr;
  /*
   * Get the address of the first line segment.
   */
  ptr = w->dial.segments;
  /*
   * calculate the center of the widget
   */
  w->dial.center_x = w->core.width/2; 
  w->dial.center_y = w->core.height/2;   

  /*
   * force the dial to be circular
   */
   w->dial.hand_length = (w->dial.center_x < w->dial.center_y) ?
			w->dial.center_x - w->dial.marker_length :
			w->dial.center_y - w->dial.marker_length;
	
  /* 
   *  Generate the segment array containing the    
   *  face of the dial.    
   */ 
  increment = RADIANS(MAX_ANGLE) / (float)(w->dial.markers); 
 angle = RADIANS(MIN_ANGLE);  
 angle = 0.0;
 for (i = 0; i < w->dial.markers;i++){   
  cosine = cos(angle);   
  sine   = sin(angle); 
  ptr->x   = (short int)(w->dial.center_x + w->dial.hand_length * sine); 
  ptr++->y   = (short int)(w->dial.center_y - w->dial.hand_length * cosine); 
  ptr->x = (short int)(w->dial.center_x +
		(w->dial.hand_length+w->dial.marker_length) * sine); 
  ptr++->y = (short int)(w->dial.center_y -
		(w->dial.hand_length+w->dial.marker_length) * cosine); 
  angle += increment; 
  }  
 calculate_indicator_pos(w); 
 calculate_arrow1_pos(w); 
 calculate_arrow2_pos(w); 
} 


static void
calculate_indicator_pos(XsDialWidget w)
{
  double   normalized_pos, angle, x, y;
  Position length;
  /*
   * Make the indicator two pixels shorter than the  
   * inner edge of the markers.
   */
  length = w->dial.hand_length - 2; 
   /*
    * Normalize the indicator position to lie between zero
    * and 1, and then convert it to an angle.
    */
  normalized_pos = (w->dial.position - w->dial.minimum)/
                 (float)(w->dial.maximum - w->dial.minimum);
  angle = RADIANS(MIN_ANGLE + MAX_ANGLE  * normalized_pos);  
   /*
    * Find the x,y coordinates of the tip of the indicator.   
    */ 
  x = w->dial.center_x + length * sin(angle);
  y = w->dial.center_y - length * cos(angle);
  w->dial.indicator_x = (Position)(x + .5); // round to nearest int
  w->dial.indicator_y = (Position)(y + .5); // round to nearest int
} 

static void
calculate_arrow1_pos(XsDialWidget w)
{
  double   normalized_pos, angle, x, y;
  Position length;
  /*
   * Make the indicator two pixels shorter than the  
   * inner edge of the markers.
   */
  length = w->dial.hand_length - 2; 
   /*
    * Normalize the indicator position to lie between zero
    * and 1, and then convert it to an angle.
    */
  normalized_pos = (w->dial.arrow1Position - w->dial.minimum)/
                 (float)(w->dial.maximum - w->dial.minimum);
  angle = RADIANS(MIN_ANGLE + MAX_ANGLE  * normalized_pos);  
   /*
    * Find the x,y coordinates of the tip of the indicator.   
    */ 
  x = w->dial.center_x + .75*length * sin(angle);
  y = w->dial.center_y - .75*length * cos(angle);
  w->dial.arrow1_x1 = (Position)(x + .5); // round to nearest int
  w->dial.arrow1_y1 = (Position)(y + .5); // round to nearest int

  w->dial.arrow1_x2 = w->dial.center_x;
  w->dial.arrow1_y2 = w->dial.center_y;
/*
  angle = RADIANS((MIN_ANGLE + MAX_ANGLE  * normalized_pos) + 180.);  
  x = w->dial.center_x + .75*length * sin(angle);
  y = w->dial.center_y - .75*length * cos(angle);
  w->dial.arrow1_x2 = (Position)(x + .5); // round to nearest int
  w->dial.arrow1_y2 = (Position)(y + .5); // round to nearest int
*/
} 

static void
calculate_arrow2_pos(XsDialWidget w)
{
  double   normalized_pos, angle, x, y;
  Position length;
  /*
   * Make the indicator two pixels shorter than the
   * inner edge of the markers.
   */
  length = w->dial.hand_length - 2;
   /*
    * Normalize the indicator position to lie between zero
    * and 1, and then convert it to an angle.
    */
  normalized_pos = (w->dial.arrow2Position - w->dial.minimum)/
                 (float)(w->dial.maximum - w->dial.minimum);
  angle = RADIANS(MIN_ANGLE + MAX_ANGLE  * normalized_pos);
   /*
    * Find the x,y coordinates of the tip of the indicator.
    */
  x = w->dial.center_x + .75*length * sin(angle);
  y = w->dial.center_y - .75*length * cos(angle);
  w->dial.arrow2_x1 = (Position)(x + .5); // round to nearest int
  w->dial.arrow2_y1 = (Position)(y + .5); // round to nearest int

  w->dial.arrow2_x2 = w->dial.center_x;
  w->dial.arrow2_y2 = w->dial.center_y;
}


static void
Redisplay(Widget widget, XEvent *event, Region region)
{
  XsDialWidget  w = (XsDialWidget)widget;

  if(w->core.visible){
    /*
     * Set the clip masks in all graphics contexts.
     */
/*
    XSetRegion(XtDisplay(w), w->dial.dial_GC, region);
    XSetRegion(XtDisplay(w), w->dial.indicator_GC, region);
*/
    /*
     * Draw the markers used for the dial face.
     */
    XDrawSegments(XtDisplay(w), XtWindow(w),
                 w->dial.dial_GC, 
                 (XSegment *)w->dial.segments,
                 w->dial.markers);
    /*
     * Draw the indicator at its current position.
     */
    XDrawLine(XtDisplay(w), XtWindow(w),
              w->dial.indicator_GC, 
              w->dial.center_x, 
              w->dial.center_y,   
              w->dial.indicator_x,  
              w->dial.indicator_y);   

    if (w->dial.arrow1Visible) {
	XDrawLine(XtDisplay(w), XtWindow(w),
	      w->dial.arrow1_GC,
	      w->dial.arrow1_x1,
	      w->dial.arrow1_y1,
	      w->dial.arrow1_x2,
	      w->dial.arrow1_y2);

    }

    if (w->dial.arrow2Visible) {
	XDrawLine(XtDisplay(w), XtWindow(w),
	      w->dial.arrow2_GC,
	      w->dial.arrow2_x1,
	      w->dial.arrow2_y1,
	      w->dial.arrow2_x2,
	      w->dial.arrow2_y2);

    }
    }
 } 

static Boolean
SetValues(XsDialWidget current, XsDialWidget request, XsDialWidget new_index)
{
  XGCValues  values;
  XtGCMask   valueMask;
  Boolean    redraw;
  Boolean    redraw_indicator;
  Boolean    redraw_arrow1;
  Boolean    redraw_arrow2;

  redraw = FALSE;
  redraw_indicator = FALSE;
  redraw_arrow1 = FALSE;
  redraw_arrow2 = FALSE;
  /*
   * Make sure the new_index dial values are reasonable.
   */

  if (new_index->dial.minimum >= new_index->dial.maximum) {
    XtWarning ("Minimum must be less than Maximum");
    new_index->dial.minimum = 0;
    new_index->dial.maximum = 360;
  }
/*
  if (new_index->dial.position > new_index->dial.maximum) {
    XtWarning("Dial position is greater than the Dial Maximum");
    new_index->dial.position =  new_index->dial.maximum;
  }
  if (new_index->dial.position < new_index->dial.minimum) {
    XtWarning("Dial position is less than the Dial Minimum");
    new_index->dial.position =  new_index->dial.minimum;
  }
*/
  /*
   * If the indicator color or background color 
   * has changed, generate the GC's.
   */
 if (new_index->dial.indicator_color!=current->dial.indicator_color||
  new_index->core.background_pixel != current->core.background_pixel ||
  new_index->dial.arrow1Color != current->dial.arrow1Color ||
  new_index->dial.arrow2Color != current->dial.arrow2Color){
    valueMask = GCForeground | GCBackground;
    values.foreground = new_index->dial.indicator_color;
    values.background = new_index->core.background_pixel;
    new_index->dial.indicator_GC = XtGetGC((Widget)new_index, valueMask,&values);
    values.foreground = new_index->dial.arrow1Color;
    new_index->dial.arrow1_GC = XtGetGC ((Widget)new_index, valueMask, &values);  
    values.foreground = new_index->dial.arrow2Color;
    new_index->dial.arrow2_GC = XtGetGC ((Widget)new_index, valueMask, &values);  

    values.foreground = new_index->core.background_pixel;
    values.background = new_index->dial.indicator_color;
    new_index->dial.inverse_GC = XtGetGC((Widget)new_index, valueMask, &values);
    values.background = new_index->dial.arrow1Color;
    new_index->dial.inverse_arrow1_GC = XtGetGC ((Widget)new_index, valueMask, &values);   
    values.background = new_index->dial.arrow2Color;
    new_index->dial.inverse_arrow2_GC = XtGetGC ((Widget)new_index, valueMask, &values);   
    redraw_indicator = TRUE;     
  }
  /*
   * If the marker color has changed, generate the GC.
   */
  if (new_index->dial.foreground != current->dial.foreground){
    valueMask = GCForeground | GCBackground;
    values.foreground = new_index->dial.foreground;
    values.background = new_index->core.background_pixel;
    new_index->dial.dial_GC = XtGetGC ((Widget)new_index, valueMask, &values);   
    redraw = TRUE;     
  }
  /*
   * If the indicator position has changed, or if the min/max
   * values have changed, recompute the indicator coordinates.
   */
  if (new_index->dial.position != current->dial.position ||
      new_index->dial.minimum != current->dial.minimum ||
      new_index->dial.maximum != current->dial.maximum){
    calculate_indicator_pos(new_index);
    redraw_indicator = TRUE;
  }
  if (new_index->dial.arrow1Visible ||
      new_index->dial.arrow1Position != current->dial.arrow1Position ||
      new_index->dial.arrow2Visible ||
      new_index->dial.arrow2Position != current->dial.arrow2Position ||
      new_index->dial.minimum != current->dial.minimum ||
      new_index->dial.maximum != current->dial.maximum){
    calculate_indicator_pos(new_index);
    redraw_indicator = TRUE;
    if (new_index->dial.arrow1Visible) {
    	calculate_arrow1_pos(new_index);
    	redraw_arrow1 = TRUE;
    }
    if (new_index->dial.arrow2Visible) {
    	calculate_arrow2_pos(new_index);
    	redraw_arrow2 = TRUE;
    }
  }
  if (!new_index->dial.arrow1Visible && current->dial.arrow1Visible &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(current), XtWindow(current),
              current->dial.inverse_GC, 
	      current->dial.arrow1_x1,
	      current->dial.arrow1_y1,
	      current->dial.arrow1_x2,
	      current->dial.arrow1_y2);
  }
  if (!new_index->dial.arrow2Visible && current->dial.arrow2Visible &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(current), XtWindow(current),
              current->dial.inverse_GC, 
	      current->dial.arrow2_x1,
	      current->dial.arrow2_y1,
	      current->dial.arrow2_x2,
	      current->dial.arrow2_y2);
  }
  /*
   * If only the indicator needs to be redrawn and
   * the widget is realized, erase the current indicator
   * and draw the new_index one.
   */
  if(redraw_indicator && ! redraw &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(current), XtWindow(current),
              current->dial.inverse_GC, 
              current->dial.center_x, 
              current->dial.center_y,
             current->dial.indicator_x, 
              current->dial.indicator_y);    
      } 
  if(redraw_arrow1 && ! redraw &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(current), XtWindow(current),
              current->dial.inverse_GC, 
	      current->dial.arrow1_x1,
	      current->dial.arrow1_y1,
	      current->dial.arrow1_x2,
	      current->dial.arrow1_y2);
      } 
  if(redraw_arrow2 && ! redraw &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(current), XtWindow(current),
              current->dial.inverse_GC, 
	      current->dial.arrow2_x1,
	      current->dial.arrow2_y1,
	      current->dial.arrow2_x2,
	      current->dial.arrow2_y2);
      } 
  if(redraw_indicator && ! redraw &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(new_index), XtWindow(new_index),  
              new_index->dial.indicator_GC,  
              new_index->dial.center_x, 
              new_index->dial.center_y,
              new_index->dial.indicator_x,
              new_index->dial.indicator_y); 
      } 
  if(redraw_arrow1 && ! redraw &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(new_index), XtWindow(new_index),  
              new_index->dial.arrow1_GC,  
	      new_index->dial.arrow1_x1,
	      new_index->dial.arrow1_y1,
	      new_index->dial.arrow1_x2,
	      new_index->dial.arrow1_y2);
      } 
  if(redraw_arrow2 && ! redraw &&
     XtIsRealized((Widget)new_index) && new_index->core.visible){
    XDrawLine(XtDisplay(new_index), XtWindow(new_index),  
              new_index->dial.arrow2_GC,  
	      new_index->dial.arrow2_x1,
	      new_index->dial.arrow2_y1,
	      new_index->dial.arrow2_x2,
	      new_index->dial.arrow2_y2);
      } 
  return (redraw); 
} 

static void
select_dial(XsDialWidget w, XEvent *event)
{
	Position   pos;
	double     angle;
	DialCallbackStruct s;
  
	pos = w->dial.position;
	/* 
	 * Get the angle in radians.
	*/
	angle= atan2((double)(event->xbutton.y - w->dial.center_y),
                 (double)(event->xbutton.x - w->dial.center_x));

	/*
	 * Convert to degrees from the MIN_ANGLE.
	 */ 
	angle = DEGREES(angle) - (MIN_ANGLE - 90.0); 
	if (angle < 0) {
	    angle = 360.0 + angle;
	}
	/*  
	 * Convert the angle to a position. 
	 */ 
	pos = (Position)(w->dial.minimum + (angle / MAX_ANGLE * 
			     (w->dial.maximum - w->dial.minimum)));  

	s.event = event;
	s.position = (int)pos;
	/*
	 * Use the position as the call_data to the callback list. 
	 */  
	XtCallCallbacks ((Widget)w, XtNselect, (XtPointer)&s); 
} 
