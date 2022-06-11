/* ****************************************************
 * DialP.h: Private header file for Dial Widget Class
 *****************************************************/


#ifndef DIALP_H
#define DIALP_H

#define MAXSEGMENTS 200

/// @cond
typedef struct _XsDialClassPart{
       int ignore;
} XsDialClassPart;

typedef struct _XsDialClassRec{
   CoreClassPart         core_class;
   XsDialClassPart       dial_class;
} XsDialClassRec;

extern XsDialClassRec XsdialClassRec;

typedef struct _XsDialPart {
   Pixel     indicator_color;  /* Color of the           */
   Pixel     foreground;       /*  indicator and markers */
   Pixel     arrow1Color;
   Boolean   arrow1Visible;
   Pixel     arrow2Color;
   Boolean   arrow2Visible;
   int       minimum;          /* minimum value          */
   int       maximum;          /* maximum value          */
   int       markers;          /* number of marks        */
   Dimension marker_length;    /* in pixels              */
   Position  position;         /* indicator position     */
   Position  indicator_x;      /* x,y position of tip    */
   Position  indicator_y;      /*     of the indicator   */
   Position  center_x;         /* coords of the          */
   Position  center_y;         /*     dial center        */
   Position  arrow1Position;
   Position  arrow1_x1;
   Position  arrow1_y1;
   Position  arrow1_x2;
   Position  arrow1_y2;
   Position  arrow2Position;
   Position  arrow2_x1;
   Position  arrow2_y1;
   Position  arrow2_x2;
   Position  arrow2_y2;
   int	     hand_length;
   GC        dial_GC;          /* assorted gc's          */
   GC        indicator_GC;
   GC        inverse_GC;
   GC        arrow1_GC;
   GC        arrow2_GC;
   GC        inverse_arrow1_GC;
   GC        inverse_arrow2_GC;
   XPoint    segments[MAXSEGMENTS];
   XtCallbackList select;    /* callback list          */
} XsDialPart;

typedef struct _XsDialRec {
   CorePart          core;
   XsDialPart        dial;
} XsDialRec;

/// @endcond

#endif /* DIALP_H */
