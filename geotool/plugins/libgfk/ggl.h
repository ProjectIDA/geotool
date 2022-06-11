#ifndef _GGL_H_
#define _GGL_H_

#include <stdlib.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "libgplot.h"
#include "libdrawx.h"


typedef struct
{
	double		slowness_min;
	double		slowness_max;
	int		n_slowness;
	SegArray	s;
	SegArray	t;
	LabelArray	l;
	Matrx		m;
	int		valid;
} gglLineProjection;

typedef struct
{
	int	button;
	int	down;
	int	X;
	int	Y;
	int	scalerate;
} MousebuttonStatus;

typedef struct
{
	GLfloat		Xrot;
	GLfloat		Yrot;
	GLfloat		Zrot;
	GLfloat		DXrot;
	GLfloat		DZrot;

	GLfloat		Xtran;
	GLfloat		Ytran;
	GLfloat		Ztran;

	GLfloat		Xscal;
	GLfloat		Yscal;
	GLfloat		Zscal;
    
	float		rot_step;
	float		tran_step;
	float		scal_step;

	int		last_dist;
	int		local_color;

	Widget		parent;
	Widget		glxarea;
	GLXContext	cx;
	GLboolean	made_current;
	Display		*dpy;
	Window		win;

	MousebuttonStatus button;
} GGLStatus;

typedef struct
{
	int	blue;
	int	green;
	int	red;
} GGLColor;

typedef struct
{
	float		center_x;
	float		center_y;
	float		center_z;
	float		x_len;
	float		y_len;
	float		z_len;
	float		lhair;
	float		shair;
	char		label[256];
	float		label_h;
	float		label_w;
	GGLColor	axis_color;
	GGLColor	grid_color;
	GGLColor	label_color;
	GGLColor	bottom_color;
} GGLOrdinate;

typedef struct {
	int		enable;
	float		height;
	GGLColor	color;
} GGLSplitPlane;

typedef struct
{
	int		draw_type;  /* 1 grid , 0 solid */
	float		transZ;
	float		magnifyZ;
	int		checked_grid;
	GLuint		drawList;
	GGLOrdinate	ordinate;
	GGLSplitPlane	plane;
} GGLTopo;

typedef struct
{
	float	*x;
	float	*y;
	float	*z;
	float	oz_max, oz_min;
	float	ox_max, ox_min;
	float	oy_max, oy_min;
	float	scale_x;
	float	scale_y;
	float	scale_z;
	float	unscale_x;
	float	unscale_y;
	float	unscale_z;
	int	nx;
	int	ny;
	float	z_min;
	float	z_max;
	float	x_min;
	float	x_max;
	float	y_min;
	float	y_max;
	int	computeAll; 
} CurrentFK;


typedef struct gglWidget_s
{
	int			init_ggl;
	CurrentFK		data;
	GGLTopo			ggl_topo;
	GGLStatus		ggl_status;
	gglLineProjection	contours;
} gglWidget;


#ifndef PI
#define PI 3.14159265358979323846
#endif

void ggl_drawContours(gglWidget *ggl,int recal);
void ggl_getContours(gglLineProjection *ggl, float *fk);
void show_glxvisinfo(Display *dpy, int screen_no);
XVisualInfo *select_truecolor(Display *dpy, int screen_no, int *pisdb);
XVisualInfo *select_24bitgl(Display *dpy, int screen_no, int *pisdb);
void  ggl_SetDrawType(gglWidget *ggl,int t);
void  ggl_Redraw(gglWidget *ggl);
short ggl_isvalid(gglWidget *ggl);
void ggl_destroy(gglWidget *ggl);
gglWidget *ggl_CreateOpenGLWidget(Widget parent); 
void ggl_SetLabel(gglWidget *ggl, char *label);
void ggl_SetXYScale(gglWidget *ggl, double x_min, double x_max, double y_min,
		double y_max);
void ggl_SetScale(gglWidget * ggl, double x_min, double x_max,
		double y_min, double y_max, float fk_min, float fk_max);
void ggl_ShowOneFK(gglWidget * ggl, int x_slowness, double *x,
		int y_slowness, double *y, float * fk, int local_color);

#endif /* _GGL_H_ */
