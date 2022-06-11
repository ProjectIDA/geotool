/** \file ggl_widget.c
 *  \brief Defines OpenGL widget ggl_widget.
 */
#include "config.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#ifdef HAVE_OPENGL

#include <Xm/MainW.h>

#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

/* GLwDrawA.h can be found in one of three places: 
   <GL/GLwDrawA.h>
   <GLw/GLwDrawA.h>
   <X11/GLw/GLwDrawA.h>
   use nested ifdefs to include the proper file
*/

#ifdef HAVE_GLWDRAWA_H
#include <GL/GLwDrawA.h>  /* IRIX 5.2: Motif OpenGL drawing area widget */
#else  /* HAVE_GLWDRAWA_H */

#ifdef HAVE_GLWGLWDRAWA_H
#include <GLw/GLwDrawA.h>
#else  /* HAVE_GLWGLWDRAWA_H */

#ifdef HAVE_X11GLWGLWDRAWA_H
#include <X11/GLw/GLwDrawA.h>
#endif /* HAVE_X11GLWGLWDRAWA_H */

#endif /* HAVE_GLWGLWDRAWA_H */

#endif /* HAVE_GLWDRAWA_H */

#include "libmath.h"
#include "libstring.h"
#include "ggl.h"

#define Free(p) if(p != NULL) {free(p); p = NULL;}

static void drawLabels_X(gglWidget *ggl);
static void drawLabels_Y(gglWidget *ggl);
static void drawLabels_Z(gglWidget *ggl);
static void ggl_DrawCallback(Widget w, XtPointer data, XtPointer callData);
static void ggl_Draw(XtPointer callData);
static void drawMargin(gglWidget *ggl, float shair, double d, double h0,
		double l0, double x);
static void ggl_keyPress(gglWidget *ggl, XEvent *event);
static void regularKey(gglWidget *ggl, char *buf);
static void ggl_ButtonPress(gglWidget *ggl, XEvent *event);
static void ggl_ButtonRelease(gglWidget *ggl, XEvent *event);

static void calcNormal(float v[3][3], float out[3]);
/*static GLuint eventListNear, eventListFar;  eventListSurface; */

#define Color_n 20
static int r_color[] ={238,   119,  71,  38,  44, 
                        44,   44,   44,  32,  32,
                        210,  210,  210, 231, 237,
                        237,  237,  255, 255, 255, 255};
static int g_color[] ={232,   133,  150, 183, 255, 
                        208,  208,  233, 233, 233,
                        233,  201,  201, 201, 158,
                        158,  158,  0,   0,   0,  0};
static int b_color[] ={229, 208, 237, 237, 255,  
                        26,   0,    0,   22,  22,  
                        11,   11,   18,  18,  18, 
                        0,    0,    0,   0,   0, 0,0};

int BitmapFont(Widget w, XFontStruct *font, char *string,char **bitmap,
    int *bitmap_width, int *bitmap_height);


/* font resource */
/* not used
static XtResource res[] =
{
	{"openglFont", "OpenglFont", XtRFontStruct, sizeof(XFontStruct *),
	    0, XtRString,
	    "-adobe-courier-bold-r-normal--18-180-75-75-m-110-iso8859-1"},
	{"lucidFont", "lucidFont", XtRFontStruct, sizeof(XFontStruct *),
	    0, XtRString,
	    "-b&h-lucidasans-bold-i-normal-sans-0-0-0-0-p-0-iso8859-1"},
};
*/

/* functions script */

short
ggl_isvalid(gglWidget *ggl)
{
    if(ggl->ggl_status.glxarea)
        return 1;
    else 
        return 0;
}

void
ggl_destroy(gglWidget *ggl)
{
    XtDestroyWidget(ggl->ggl_status.glxarea);
/*    ggl->ggl_status.glxarea = NULL;
    ggl_FreeFK(ggl); */
}

void
ggl_SetLabel(gglWidget * ggl,char *label)
{
    if(label)
        stringcpy(ggl->ggl_topo.ordinate.label, label,
		sizeof(ggl->ggl_topo.ordinate.label));
}

void
ggl_SetDrawType(gglWidget *ggl,int t)
{
    if(t)ggl->ggl_topo.draw_type = 1;
    else ggl->ggl_topo.draw_type = 0;
}

static float
mpo(float o_min,float o_max, float mp_x, float mp_min,float mp_max)
{
    if(mp_min==mp_max)return 0.0;
    else return o_min+(o_max-o_min)*(mp_x-mp_min)/(mp_max-mp_min);
}


/* draw */
static void
ggl_DrawSurface(gglWidget *ggl)
{
    int i,j;
    float local_zmin=0., local_zmax=0.;

    if(!ggl->data.x || !ggl->data.y || !ggl->data.z) return;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(0,0,ggl->ggl_topo.transZ);  
    glScalef (1.0, 1.0, ggl->ggl_topo.magnifyZ);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glPushAttrib(GL_LIGHTING_BIT);
    glEnable(GL_CULL_FACE);
    glPointSize(5.0);
    glDisable(GL_LIGHTING);  
/*  glEnable(GL_LIGHTING);  */
    glColor3f(1.0, 1.0, 0.0);

    switch(ggl->ggl_topo.draw_type)
    {
        case 1:
	    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            break;
        case 0:
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            break;
        default:
            break;
    }

    if(ggl->ggl_status.local_color)
    {
	int n = ggl->data.nx * ggl->data.ny;
	if(n > 0) {
	    local_zmin = ggl->data.z[0];
	    local_zmax = ggl->data.z[0];
	}
	for(i = 0; i < n; i++) {
	    if(ggl->data.z[i] < local_zmin) local_zmin = ggl->data.z[i];
	    if(ggl->data.z[i] > local_zmax) local_zmax = ggl->data.z[i];
	}
    }
    else {
	local_zmin = ggl->data.oz_min;
	local_zmax = ggl->data.oz_max;
    }
    
    for(j = 0; j < ggl->data.ny-1; j++)
    {
	int j1 = j;
	int j2 = j+1;
        float y1 = ggl->data.y[j+1];
	float y2 = ggl->data.y[j];
	float y3 = y1;
	float y4 = y2;

 	glBegin(GL_TRIANGLE_STRIP); 

	for(i = 0; i < ggl->data.nx-1; i++) { 
	    int i1 = i;
	    int i2 = i+1;

            float x1 = ggl->data.x[i];
            float x2 = x1;
            float x3 = ggl->data.x[i+1];
            float x4 = x3;
	    float z1, z2, z3, z4;
	    float v[3][3], norm[3];
	    int color;

	    z1 = ggl->data.z[i1+j2*ggl->data.nx];
	    z2 = ggl->data.z[i1+j1*ggl->data.nx];
	    z3 = ggl->data.z[i2+j2*ggl->data.nx];
	    z4 = ggl->data.z[i2+j1*ggl->data.nx];

	    v[0][0] = x1; v[0][1] = y1; v[0][2] = z1;
	    v[1][0] = x2; v[1][1] = y2; v[1][2] = z2;
	    v[2][0] = x3; v[2][1] = y3; v[2][2] = z3;
	    calcNormal(v, norm);
	    glNormal3f(norm[0], norm[1], norm[2]);

	    if(i == 0) {
		color = (int)mpo((float)0, (float)Color_n, z1,
					local_zmin, local_zmax);
		if(color < 0) color = 0; 
		if(color > Color_n) color = Color_n;
		glColor3ub(r_color[color], g_color[color], b_color[color]);
            
	    	glVertex3f(x1, y1, z1);
		color = (int)mpo((float)0, (float)Color_n, z2,
					local_zmin, local_zmax);
		if(color < 0) color = 0; 
		if(color > Color_n) color = Color_n;
	        glColor3ub(r_color[color], g_color[color], b_color[color]);
       		glVertex3f(x2, y2, z2);
	    }

	    color = (int)mpo((float)0, (float)Color_n, z3,
				local_zmin, local_zmax);
	    if(color < 0) color = 0; 
	    if(color > Color_n) color = Color_n;
	    glColor3ub(r_color[color], g_color[color], b_color[color]);

	    glVertex3f(x3, y3, z3);

	    color = (int)mpo((float)0, (float)Color_n, z4,
				local_zmin, local_zmax);
	    if(color < 0) color = 0; 
	    if(color > Color_n) color = Color_n;
	    glColor3ub(r_color[color], g_color[color], b_color[color]);

	    v[0][0] = x2; v[0][1] = y2; v[0][2] = z2;
	    v[1][0] = x3; v[1][1] = y3; v[1][2] = z3;
	    v[2][0] = x4; v[2][1] = y4; v[2][2] = z4;
	    calcNormal(v, norm);
	    glNormal3f(norm[0], norm[1], norm[2]);
	    glVertex3f(x4, y4, z4);
        }
	glEnd();
    }  
    glDisable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glColor3ub(r_color[0], g_color[0], b_color[0]);

    ggl_drawContours(ggl,0); 

    glPopMatrix();
}

static void
ggl_DrawSimpleText(gglWidget *ggl, float x_rot, float y_rot, float z_rot,
		GLfloat x, GLfloat y, GLfloat z, float offset, char *text)
{
    int i;
    float x1, y1, x2, y2, xmax=0.;
    DrawStruct d;
    SegArray s;

    glPushMatrix();
    glTranslatef(x,y,z);  
    
    glRotatef(-x_rot*180/PI, 1.0, 0.0, 0.0);
    glRotatef(-y_rot*180/PI, 0.0, 1.0, 0.0);
    glRotatef(-z_rot*180/PI, 0.0, 0.0, 1.0);

    SetDrawArea(&d, -10000, -10000, 10000, 10000);
    SetScale(&d, -1., -1., 1., 1.);

    d.s = &s;
    d.s->segs = NULL;
    d.s->size_segs = 0;
    d.s->nsegs = 0;

    mapalf(&d, 0, 0, 700., 0., 1, text);

    glLineWidth(2.0); 

    glBegin(GL_LINES);

	if(s.nsegs > 0) xmax = s.segs[0].x1;
	for(i = 0; i < s.nsegs; i++) {
	    if(s.segs[i].x1 > xmax) xmax = s.segs[i].x1;
	    if(s.segs[i].x2 > xmax) xmax = s.segs[i].x2;
	}
	for(i = 0; i < s.nsegs; i++) {
	    x1 = (s.segs[i].x1 - xmax)/10000. - offset;
	    x2 = (s.segs[i].x2 - xmax)/10000. - offset;
	    y1 = s.segs[i].y1/10000.;
	    y2 = s.segs[i].y2/10000.;
	    glVertex3f(x1, 0., y1);
	    glVertex3f(x2, 0., y2);
	}

    glEnd();

    glPopMatrix();

    Free(s.segs);
}

static void
ggl_drawPlane(gglWidget *ggl)
{
    char label[256];

    glPushMatrix();

    glTranslatef(ggl->ggl_topo.ordinate.center_x,
		ggl->ggl_topo.ordinate.center_y,
		ggl->ggl_topo.ordinate.center_z + ggl->ggl_topo.transZ);  
    glScalef(1.0, 1.0, ggl->ggl_topo.magnifyZ);

    if(ggl->ggl_topo.plane.enable == 1)
    {
       	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glColor3ub(ggl->ggl_topo.plane.color.red,
            ggl->ggl_topo.plane.color.green,
            ggl->ggl_topo.plane.color.blue);
    
        glBegin(GL_POLYGON);
	glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len,
		ggl->ggl_topo.plane.height);
	glVertex3f(0.0, 0.0, ggl->ggl_topo.plane.height);
	glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0,
		ggl->ggl_topo.plane.height);
	glVertex3f(ggl->ggl_topo.ordinate.x_len,
		-ggl->ggl_topo.ordinate.y_len, ggl->ggl_topo.plane.height);
        glEnd();

        glColor3ub(ggl->ggl_topo.ordinate.label_color.red,
            ggl->ggl_topo.ordinate.label_color.green,
        ggl->ggl_topo.ordinate.label_color.blue);
        snprintf(label, 256, "%.2f",
		(ggl->ggl_topo.plane.height-ggl->data.oz_min)
            *(ggl->data.z_max - ggl->data.z_min)/
            (ggl->data.oz_max-ggl->data.oz_min) + ggl->data.z_min);
        ggl_DrawSimpleText(ggl, 0.0, 0.0, -PI/8.0,
		ggl->ggl_topo.ordinate.x_len + 0.3, 0.0,
		ggl->ggl_topo.plane.height - 0.06, -.2, label);
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPopMatrix();

}

static void
ggl_DrawOrdinate(gglWidget *ggl)
{
    glPushMatrix();

    glTranslatef(ggl->ggl_topo.ordinate.center_x,
        ggl->ggl_topo.ordinate.center_y,
        ggl->ggl_topo.ordinate.center_z + ggl->ggl_topo.transZ);  
    glScalef (1.0, 1.0, ggl->ggl_topo.magnifyZ);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    /* glColor3ub */

    glColor3ub(ggl->ggl_topo.ordinate.axis_color.red,
        ggl->ggl_topo.ordinate.axis_color.green,
        ggl->ggl_topo.ordinate.axis_color.blue);
    glLineWidth(2.0); 
    
    glBegin(GL_LINE_STRIP);
        glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len, 0.0);
        glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len,
		ggl->ggl_topo.ordinate.z_len);
        glVertex3f(0.0, 0.0, ggl->ggl_topo.ordinate.z_len);
        glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0,
		ggl->ggl_topo.ordinate.z_len);
        glVertex3f(+ggl->ggl_topo.ordinate.x_len, 0.0, 0.0);
    glEnd();
    
    glBegin(GL_LINE_STRIP);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, 0.0,ggl->ggl_topo.ordinate.z_len);
    glEnd();

    drawLabels_Z(ggl);
    drawLabels_X(ggl);
    drawLabels_Y(ggl);

    /* finish draw grid ordinates */    
    glDisable(GL_LINE_STIPPLE);

/*
    if(!ggl->contours.valid)
    {
        glColor3ub(ggl->ggl_topo.ordinate.bottom_color.red,
            ggl->ggl_topo.ordinate.bottom_color.green,
            ggl->ggl_topo.ordinate.bottom_color.blue);
       	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else {
        glLineWidth(2.0); 
       	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }   
*/
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glPushAttrib(GL_LIGHTING_BIT);
    glEnable(GL_CULL_FACE);
    glPointSize(5.0);
    glDisable(GL_LIGHTING);  

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glColor3ub(ggl->ggl_topo.ordinate.bottom_color.red,
            ggl->ggl_topo.ordinate.bottom_color.green,
            ggl->ggl_topo.ordinate.bottom_color.blue);
    
    glBegin(GL_POLYGON);
        glVertex3f(ggl->ggl_topo.ordinate.x_len, -ggl->ggl_topo.ordinate.y_len,
			0.0);
        glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0, 0.0);
        glVertex3f(0.0, 0.0, 0.0);
        glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len, 0.0);
    glEnd();

    glColor3ub(ggl->ggl_topo.ordinate.label_color.red,
            ggl->ggl_topo.ordinate.label_color.green,
            ggl->ggl_topo.ordinate.label_color.blue);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glPopMatrix();
}

static void
drawLabels_Z(gglWidget *ggl)
{
    float shair;
    double nx[20],subx[20];
    int ndigit, ndeci,nlabels,n = 0,xdigit, xdeci,i,j;
    char label[120];
    GLint factor = 4;   /* Stippling factor */
    GLushort pattern = 0x5555;  /* Stipple pattern */
    double x,x1,d=0.,h0,l0;

    shair = ggl->ggl_topo.ordinate.shair;

    /* determine the Z axis label numbers */
    h0 = mpo(ggl->data.z_min, ggl->data.z_max,
            ggl->data.oz_min + ggl->ggl_topo.ordinate.z_len,
            ggl->data.oz_min, ggl->data.oz_max);
    l0 = mpo(ggl->data.z_min, ggl->data.z_max, ggl->data.oz_min,
            ggl->data.oz_min, ggl->data.oz_max);
    nicex(l0, h0, 4, 6, &nlabels, nx, &ndigit, &ndeci);
    nicex(nx[0], nx[1], 4, 6, &n, subx, &xdigit, &xdeci); 

    /* draw label for axis */

    glLineWidth(1.0); 
    
    /* draw -y_len to 0.0 to x_len */
    ftoa(nx[0], ndeci, 1, label, 120);

    x = mpo(0.0,ggl->ggl_topo.ordinate.z_len,nx[0],l0,h0);

    ggl_DrawSimpleText(ggl, 0.0, 0.0, -PI/8.0,
		0., -ggl->ggl_topo.ordinate.y_len, x - 0.06, .1, label);

    glEnable(GL_LINE_STIPPLE);
    glLineStipple(factor, pattern);

    glBegin(GL_LINE_STRIP);
    glVertex3f(0.0, 0.0 , x);
    glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0, x);
    glEnd();        

    glBegin(GL_LINE_STRIP);
    glVertex3f(0.0, 0.0 , x);
    glVertex3f(0.0, -ggl->ggl_topo.ordinate.x_len, x);
    glEnd();        
    glDisable(GL_LINE_STIPPLE);

    for(j = 1; j < nlabels; j++)
    {
        x = mpo(0.0, ggl->ggl_topo.ordinate.z_len, nx[j], l0, h0);
        x1 = mpo(0.0, ggl->ggl_topo.ordinate.z_len,nx[j-1], l0, h0);
	glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3f(0.0, 0.0, x);
        glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0, x);
        glEnd();        

        glBegin(GL_LINE_STRIP);
        glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len, x);
        glVertex3f(0.0, 0.0, x);
        glEnd();        
        glDisable(GL_LINE_STIPPLE);

        glColor3ub(ggl->ggl_topo.ordinate.label_color.red,
                ggl->ggl_topo.ordinate.label_color.green,
                ggl->ggl_topo.ordinate.label_color.blue);
        ftoa(nx[j], ndeci, 1, label, 120);
        ggl_DrawSimpleText(ggl,0.0, 0.0, -PI/8.0,
		0., -ggl->ggl_topo.ordinate.y_len, x - 0.06, .1, label);

        glColor3ub(ggl->ggl_topo.ordinate.grid_color.red,
                ggl->ggl_topo.ordinate.grid_color.green,
                ggl->ggl_topo.ordinate.grid_color.blue);

        if(n > 2) {
            d = ggl->ggl_topo.ordinate.z_len*((nx[j] - nx[j-1])/(n-1))/(h0-l0);
            for(i = 0; i < n-2; i++)
	    {
                glBegin(GL_LINE_STRIP);
                glVertex3f(ggl->ggl_topo.ordinate.x_len - shair, 0.0 ,
				x1+d*(i+1));
                glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0, x1+d*(i+1));
                glEnd();        
            
                glBegin(GL_LINE_STRIP);
                glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len, x1+d*(i+1));
                glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len + shair,
				x1+d*(i+1));
                glEnd();        
            }
        }
    }    

    drawMargin(ggl, shair, d, h0, l0, x);
}

static void
drawMargin(gglWidget *ggl, float shair, double d, double h0, double l0,double x)
{
    int i;
    double x1;

    /* draw the margin */
    x1 = mpo(0.0, ggl->ggl_topo.ordinate.z_len, h0, l0, h0);

    i = 1;
    while((x+i*d) < x1)
    {
	glBegin(GL_LINE_STRIP);
            glVertex3f(ggl->ggl_topo.ordinate.x_len - shair, 0.0 , x+d*i);
            glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0, x+d*i);
        glEnd();        
            
	glBegin(GL_LINE_STRIP);
	    glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len, x+d*i);
	    glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len + shair, x+d*i);
	glEnd();        
	i++;
    }

    x1 = mpo(0.0, ggl->ggl_topo.ordinate.z_len, l0, l0, h0);

    i = 1;
    while((x1 - d*i) > 0.0)
    {
	glBegin(GL_LINE_STRIP);
	    glVertex3f(ggl->ggl_topo.ordinate.x_len - shair, 0.0 , x1 - d*i);
	    glVertex3f(ggl->ggl_topo.ordinate.x_len, 0.0, x1 - d*i);
	glEnd();        
            
	glBegin(GL_LINE_STRIP);
	    glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len, x1 - d*i);
	    glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len + shair, x1 - d*i);
	glEnd();        
	i++;
    }
}

static void
drawLabels_X(gglWidget *ggl)
{
    float lhair,shair;
    double nx[20],subx[20];
    int ndigit, ndeci,nlabels,n = 0,xdigit, xdeci,i,j;
    char label[120];
    double x,x1,d,h0,l0;

    lhair = ggl->ggl_topo.ordinate.lhair;
    shair = ggl->ggl_topo.ordinate.shair;

    /* draw x_len to 0.0 to z_len */
    h0 = mpo(ggl->data.y_min, ggl->data.y_max,
            ggl->ggl_topo.ordinate.y_len + ggl->data.oy_min,
            ggl->data.oy_min, ggl->data.oy_max);
    l0 = mpo(ggl->data.y_min, ggl->data.y_max, ggl->data.oy_min,
            ggl->data.oy_min, ggl->data.oy_max);
    nicex(l0, h0, 4, 6, &nlabels, nx, &ndigit, &ndeci);
    nicex(nx[0], nx[1], 4, 6, &n, subx, &xdigit, &xdeci); 
    ftoa(nx[0], ndeci, 1, label, 120);

    x = mpo(0.0, ggl->ggl_topo.ordinate.y_len, nx[0], l0, h0);

    ggl_DrawSimpleText(ggl, 0.0, 0.0, 0.0,
		ggl->ggl_topo.ordinate.x_len + 2*lhair + 0.15,
		-ggl->ggl_topo.ordinate.y_len + x, -.06, -.11, label);

    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_STRIP);
    glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len + x, 0.0);
    glVertex3f(0.0, -ggl->ggl_topo.ordinate.y_len + x,
		ggl->ggl_topo.ordinate.z_len);
    glEnd();        
    glDisable(GL_LINE_STIPPLE);

    glBegin(GL_LINE_STRIP);
        glVertex3f(ggl->ggl_topo.ordinate.x_len,
            -ggl->ggl_topo.ordinate.y_len + x, 0.0);
        glVertex3f(ggl->ggl_topo.ordinate.x_len + lhair,
            -ggl->ggl_topo.ordinate.y_len + x, 0.0);
    glEnd();        
    
    for(j = 1; j < nlabels; j++)
    {
        x = mpo(0.0, ggl->ggl_topo.ordinate.y_len, nx[j], l0, h0);
        x1 = mpo(0.0, ggl->ggl_topo.ordinate.y_len, nx[j-1], l0, h0);

        glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3f(0.0 , -ggl->ggl_topo.ordinate.y_len + x, 0.0);
        glVertex3f(0.0 , -ggl->ggl_topo.ordinate.y_len + x,
		ggl->ggl_topo.ordinate.z_len);
        glEnd();        
        glDisable(GL_LINE_STIPPLE);

        glColor3ub(ggl->ggl_topo.ordinate.label_color.red,
		ggl->ggl_topo.ordinate.label_color.green,
		ggl->ggl_topo.ordinate.label_color.blue);

        ftoa(nx[j], ndeci, 1, label, 120);
        ggl_DrawSimpleText(ggl, 0.0, 0.0, 0.0,
		ggl->ggl_topo.ordinate.x_len + 2*lhair + 0.15,
		-ggl->ggl_topo.ordinate.y_len + x, -.06, -.11, label);

        glColor3ub(ggl->ggl_topo.ordinate.grid_color.red,
		ggl->ggl_topo.ordinate.grid_color.green,
		ggl->ggl_topo.ordinate.grid_color.blue);


        glBegin(GL_LINE_STRIP);
        glVertex3f(ggl->ggl_topo.ordinate.x_len,
		-ggl->ggl_topo.ordinate.y_len + x, 0.0);
        glVertex3f(ggl->ggl_topo.ordinate.x_len + lhair,
		-ggl->ggl_topo.ordinate.y_len + x, 0.0);
        glEnd();     
           

        if(n > 2)
	{
	    d = ggl->ggl_topo.ordinate.y_len*((nx[j] - nx[j-1])/(n-1))/(h0-l0);
            for(i = 0; i < n-2; i++)
	    {
                glBegin(GL_LINE_STRIP);
                glVertex3f(ggl->ggl_topo.ordinate.x_len,
                    -ggl->ggl_topo.ordinate.y_len + x1 + d*(i+1), 0.0);
                glVertex3f(ggl->ggl_topo.ordinate.x_len + shair,
                    -ggl->ggl_topo.ordinate.y_len + x1 + d*(i+1), 0.0);
                glEnd();
            }
        }
    }    
}

static void
drawLabels_Y(gglWidget *ggl)
{
    float lhair,shair;
    double nx[20],subx[20];
    int ndigit, ndeci,nlabels,n = 0,xdigit, xdeci,i,j;
    char label[120];
    double x,x1,d,h0,l0;

    lhair = ggl->ggl_topo.ordinate.lhair;
    shair = ggl->ggl_topo.ordinate.shair;

    /* draw 0.0 to x_len to z_len */
    h0 = mpo(ggl->data.x_min, ggl->data.x_max,
            ggl->data.ox_min + ggl->ggl_topo.ordinate.x_len,
            ggl->data.ox_min, ggl->data.ox_max);
    l0 = mpo(ggl->data.x_min, ggl->data.x_max, ggl->data.ox_min,
            ggl->data.ox_min, ggl->data.ox_max);
    nicex(l0, h0, 4, 6, &nlabels, nx, &ndigit, &ndeci);
    nicex(nx[0], nx[1], 4, 6, &n, subx, &xdigit, &xdeci); 

    ftoa(nx[0], ndeci, 1, label, 120);

    x = mpo(0.0, ggl->ggl_topo.ordinate.x_len, nx[0], l0, h0);

    glEnable(GL_LINE_STIPPLE);
    glBegin(GL_LINE_STRIP);
    glVertex3f(x , 0.0, 0.0);
    glVertex3f(x , 0.0, ggl->ggl_topo.ordinate.z_len);
    glEnd();        
    glDisable(GL_LINE_STIPPLE);

    glBegin(GL_LINE_STRIP);
    glVertex3f(x, -ggl->ggl_topo.ordinate.y_len - lhair, 0.0);
    glVertex3f(x, -ggl->ggl_topo.ordinate.y_len , 0.0);
    glEnd();        

    ggl_DrawSimpleText(ggl, 0.0, 0.0, -PI*3/8,
		x, -ggl->ggl_topo.ordinate.y_len, -.06, .15, label);


    for(j = 1; j < nlabels; j++)
    {
        x = mpo(0.0, ggl->ggl_topo.ordinate.x_len, nx[j], l0, h0);
        x1 = mpo(0.0, ggl->ggl_topo.ordinate.x_len, nx[j-1], l0, h0);

	glEnable(GL_LINE_STIPPLE);
        glBegin(GL_LINE_STRIP);
        glVertex3f( x , 0.0, 0.0);
        glVertex3f( x , 0.0, ggl->ggl_topo.ordinate.z_len);
        glEnd();        
        glDisable(GL_LINE_STIPPLE);

        glColor3ub(ggl->ggl_topo.ordinate.label_color.red,
            ggl->ggl_topo.ordinate.label_color.green,
            ggl->ggl_topo.ordinate.label_color.blue);

        ftoa(nx[j], ndeci, 1, label, 120);
        ggl_DrawSimpleText(ggl, 0.0, 0.0, -PI*3/8,
		x, -ggl->ggl_topo.ordinate.y_len, -.06, .15, label);

        glColor3ub(ggl->ggl_topo.ordinate.grid_color.red, \
            ggl->ggl_topo.ordinate.grid_color.green, \
            ggl->ggl_topo.ordinate.grid_color.blue);


        glBegin(GL_LINE_STRIP);
        glVertex3f(x, -ggl->ggl_topo.ordinate.y_len - lhair, 0.0);
        glVertex3f(x, -ggl->ggl_topo.ordinate.y_len , 0.0);
        glEnd();        


        if(n > 2) {
            d = ggl->ggl_topo.ordinate.x_len*((nx[j] - nx[j-1])/(n-1))/(h0-l0);
            for(i = 0;i < n-2; i++) {
                glBegin(GL_LINE_STRIP);
                glVertex3f(x1 + d*(i+1), -ggl->ggl_topo.ordinate.y_len - shair,
			0.0);
                glVertex3f(x1 + d*(i+1), -ggl->ggl_topo.ordinate.y_len , 0.0);
                glEnd();
            }
        }
    } 
}

static void
ggl_smpDrawSurface(gglWidget * ggl)
{

    /* lighting */
    GLfloat gray[4] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat matcolor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    GLfloat lightPos[4] = {-5.f, 5.f, 5.f, 1.f};
        
    glEnable(GL_LIGHTING);
        
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, white);
    glLightfv(GL_LIGHT0, GL_AMBIENT, gray);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matcolor);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
    glEnable(GL_COLOR_MATERIAL);

    glEnable(GL_LIGHT0);
}


static void
ggl_ZeroStatus(gglWidget * ggl)
{
    ggl->ggl_status.Xrot = -66.0;
    ggl->ggl_status.Yrot = 0.0;
    ggl->ggl_status.Zrot = -24.0;
    ggl->ggl_status.DXrot = 1.0;
    ggl->ggl_status.DZrot = 2.5;

    ggl->ggl_status.Xtran = 0.0;
    ggl->ggl_status.Ytran = 0.0;
    ggl->ggl_status.Ztran = 0.0;

    ggl->ggl_status.Xscal = 3.0;
    ggl->ggl_status.Yscal = 3.0;
    ggl->ggl_status.Zscal = 3.0;
    
    ggl->ggl_status.rot_step = -3.0;
    ggl->ggl_status.tran_step = 0.2f;
    ggl->ggl_status.scal_step = 1.0f;

    ggl->ggl_status.last_dist = -1;
    ggl->ggl_status.local_color = 0;

    ggl->ggl_status.parent = NULL;    
    ggl->ggl_status.glxarea = NULL;
    ggl->ggl_status.cx = NULL;
    ggl->ggl_status.win = 0;
    ggl->ggl_status.dpy = NULL;
    ggl->ggl_status.made_current = GL_FALSE;
    
    ggl->ggl_status.button.scalerate = 50;
    ggl->ggl_status.button.button = 0;    

    /* initalize topo struct */
    ggl->ggl_topo.draw_type = 0;
    ggl->ggl_topo.transZ = -1.45;
    ggl->ggl_topo.magnifyZ = 1.5;

    ggl->ggl_topo.checked_grid = 0;
    ggl->ggl_topo.drawList = 0;
    
    ggl->ggl_topo.ordinate.center_x = -1.0;
    ggl->ggl_topo.ordinate.center_y = 1.0;
    ggl->ggl_topo.ordinate.center_z = 0.0;
    ggl->ggl_topo.ordinate.x_len = 2.0;
    ggl->ggl_topo.ordinate.y_len = 2.0;
    ggl->ggl_topo.ordinate.z_len = 2.2;
    ggl->ggl_topo.ordinate.lhair = 0.05;
    ggl->ggl_topo.ordinate.shair = 0.03;
    
    ggl->ggl_topo.ordinate.axis_color.blue = 255;
    ggl->ggl_topo.ordinate.axis_color.green = 255;
    ggl->ggl_topo.ordinate.axis_color.red = 255;        
    ggl->ggl_topo.ordinate.grid_color.blue = 255;
    ggl->ggl_topo.ordinate.grid_color.green = 255;
    ggl->ggl_topo.ordinate.grid_color.red = 255;
    ggl->ggl_topo.ordinate.label_color.red = 255;
    ggl->ggl_topo.ordinate.label_color.green = 255;
    ggl->ggl_topo.ordinate.label_color.blue = 255;
    ggl->ggl_topo.ordinate.label_h = 13;
    ggl->ggl_topo.ordinate.label_w = 6.0;

    ggl->ggl_topo.ordinate.bottom_color.red = 119;
    ggl->ggl_topo.ordinate.bottom_color.green = 136;	/* light slate grey */
    ggl->ggl_topo.ordinate.bottom_color.blue = 153;
    stringcpy(ggl->ggl_topo.ordinate.label,"",
		sizeof(ggl->ggl_topo.ordinate.label));
    
    ggl->ggl_topo.plane.enable = 0;
    ggl->ggl_topo.plane.height = 0.0;
    ggl->ggl_topo.plane.color.blue = 239;    
    ggl->ggl_topo.plane.color.green = 254;    
    ggl->ggl_topo.plane.color.red = 173; 
    
    ggl->data.x = NULL;
    ggl->data.y = NULL;
    ggl->data.z = NULL;
    ggl->data.z_max = 0.0;
    ggl->data.z_min = 0.0;
    ggl->data.ox_max = 1.0;
    ggl->data.ox_min = -1.0;
    ggl->data.oy_max = 1.0;
    ggl->data.oy_min = -1.0;
    ggl->data.oz_max = 2.0;
    ggl->data.oz_min = 0.0;
    ggl->data.nx = 0;
    ggl->data.ny = 0;
    ggl->data.computeAll = 0;
    ggl->data.z_max = 1.0;
    ggl->data.z_min = 0.0;
    ggl->data.x_min = -1.0;
    ggl->data.x_max = 1.0;
    ggl->data.y_min = -1.0;
    ggl->data.y_max = 1.0;

    ggl->data.unscale_x = (ggl->data.ox_max - ggl->data.ox_min)/
				(ggl->data.x_max - ggl->data.x_min);
    ggl->data.unscale_y = (ggl->data.oy_max - ggl->data.oy_min)/
				(ggl->data.y_max - ggl->data.y_min);
    ggl->data.unscale_z = (ggl->data.oz_max - ggl->data.oz_min)/
				(ggl->data.z_max - ggl->data.z_min);
    ggl->data.scale_x = (ggl->data.unscale_x != 0.)
				? 1./ggl->data.unscale_x : 1.;
    ggl->data.scale_y = (ggl->data.unscale_y != 0.)
				? 1./ggl->data.unscale_y : 1.;
    ggl->data.scale_z = (ggl->data.unscale_z != 0.)
				? 1./ggl->data.unscale_z : 1.;

    ggl->contours.s.segs = NULL;
    ggl->contours.t.segs = NULL;
    ggl->contours.l.ls = NULL;
    ggl->contours.valid = 0;    
    
    ggl->init_ggl = 0;
}

static void
ggl_InitialzieOpenGL(gglWidget *ggl)
{
    ggl->init_ggl = 1;
    ggl_ZeroStatus(ggl);
}

static void
ggl_defaultGL(gglWidget *ggl)
{
    glClearDepth(1.0);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);
}

static void
ggl_MapStateChangedCallback(Widget w, XtPointer data, XEvent *event,
			Boolean *cont)
{
    switch (event->type)
    {
	case MapNotify:
/*    	if (moving && workId != 0) workId = XtAppAddWorkProc(app, animate, NULL); */
        printf("MapNotify\n");
	break;
    case UnmapNotify:
/*	if (moving) XtRemoveWorkProc(workId); */
        printf("UnmapNOtify\n");
	break;
    }
}


static void
ggl_setProjection(int dist)
{
    switch(dist) {
	case 4: glFrustum( -1.0, 1.0, -1.0, 1.0, 10.0, 100.0 );
		break;
	case 3:	glFrustum( -.5, .5, -.5, .5, 5.0, 50.0 );
		break;
	case 2: glFrustum( -.125, .125, -.125, .125, 2.0, 20.0 );
		break;
	case 1: glFrustum( -.1, .1, -.1, .1, 1.0, 10.0 );
		break;
    }
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -70.0);
}

static void
ggl_DrawCallback(Widget w, XtPointer data, XtPointer callData)
{
    gglWidget *ggl = (gglWidget *)data;

    if(ggl->ggl_status.glxarea) {
        glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
			ggl->ggl_status.cx);
	ggl_Draw(data);
    }
}
    
static void
ggl_Draw(XtPointer data)
{

    int dist;
    gglWidget *ggl = (gglWidget *)data;
    

    /* Set clear color to blue */
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    
    /* Clear the window */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(ggl->ggl_status.Ztran < 55) dist = 4;
    else if(ggl->ggl_status.Ztran < 63) dist = 3;
    else if(ggl->ggl_status.Ztran < 68) dist = 2;
    else dist = 1;

    if(dist != ggl->ggl_status.last_dist)
    {
	ggl->ggl_status.last_dist = dist;
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	ggl_setProjection(dist);
    }

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslatef(ggl->ggl_status.Xtran, ggl->ggl_status.Ytran,
		ggl->ggl_status.Ztran);
    glScalef(ggl->ggl_status.Xscal, ggl->ggl_status.Yscal,
		ggl->ggl_status.Zscal); 
    glRotatef(ggl->ggl_status.Xrot, 1.0, 0.0, 0.0);
    glRotatef(ggl->ggl_status.Yrot, 0.0, 1.0, 0.0);
    glRotatef(ggl->ggl_status.Zrot, 0.0, 0.0, 1.0);

/*    glCallList(mapObj);
    glCallList(eventListSurface); */

    ggl_drawPlane(ggl);

    if(ggl->ggl_topo.drawList > 0) {
	glCallList(ggl->ggl_topo.drawList);
    }
    glPopMatrix();

    if(ggl->ggl_status.win) {
        glXSwapBuffers(ggl->ggl_status.dpy, ggl->ggl_status.win);
    }
}

static void
calcNormal(float v[3][3], float out[3])
{
    float v1[3], v2[3], amp;
    static const int x = 0;
    static const int y = 1;
    static const int z = 2;

    /* Calculate two vectors from the thress points */
    v1[x] = v[1][x] - v[0][x];
    v1[y] = v[1][y] - v[0][y];
    v1[z] = v[1][z] - v[0][z];

    v2[x] = v[2][x] - v[0][x];
    v2[y] = v[2][y] - v[0][y];
    v2[z] = v[2][z] - v[0][z];

    /* Take the cross product of the two vectors to get the normal vector. */
    out[x] = v1[y]*v2[z] - v1[z]*v2[y];
    out[y] = v1[z]*v2[x] - v1[x]*v2[z];
    out[z] = v1[x]*v2[y] - v1[y]*v2[x];

    amp = sqrt(out[x]*out[x] + out[y]*out[y] + out[z]*out[z]);
    out[x] /= amp;
    out[y] /= amp;
    out[z] /= amp;
}

static void
ggl_DestroyCallback(Widget w, XtPointer data, XtPointer callData)
{
    gglWidget *ggl = (gglWidget *)data;

    if(ggl->ggl_status.glxarea) {
        glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
			ggl->ggl_status.cx);
    }
    
    fprintf(stderr,"destroy ggl widget\n");
    if(ggl->ggl_status.dpy) {
	glXDestroyContext(ggl->ggl_status.dpy,ggl->ggl_status.cx);
    }
    ggl_ZeroStatus(ggl);
}

static void
ggl_ResizeCallback(Widget w, XtPointer data, XtPointer callData)
{
    Dimension       width, height;
    gglWidget       *ggl = (gglWidget *)data;

    if(ggl->ggl_status.glxarea) {
        glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
			ggl->ggl_status.cx);
    }

    if(ggl->ggl_status.made_current)
    {
    	XtVaGetValues(w, XmNwidth, &width, XmNheight, &height, NULL);
	    glViewport(0, 0, (GLint) width, (GLint) height);
/*
         Reset projection matrix stack
        glMatrixMode(GL_PROJECTION);
        
         Establish clipping volume ( left, right, bottom, top, near, far )
        if(width<height)
            glOrtho(-nRange,nRange, -nRange *height/width,
                 nRange * height / width, -nRange,nRange);
        else
            glOrtho(-nRange*height/width,nRange* height / width,
                -nRange , nRange , -nRange,nRange);
*/    
    }
}

static void
ggl_InputCallback(Widget wid, XtPointer client_data, XEvent *event,
			Boolean *con)
{
    gglWidget *ggl = (gglWidget *)client_data;

    if (event == NULL) return;

    if(ggl->ggl_status.glxarea) {
        glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
			ggl->ggl_status.cx);
    }

    if(event->type == KeyPress)
    {
	ggl_keyPress(ggl, event);
    }
    else if(event->type == ButtonPress)
    {
	ggl_ButtonPress(ggl, event);
    }
    else if(event->type == ButtonRelease)
    {
	ggl_ButtonRelease(ggl, event);
    }
}

static void
ggl_keyPress(gglWidget *ggl, XEvent *event)
{
    char buf[1];
    KeySym keysym;

    buf[0] = '\0';
    XLookupString(&(event->xkey), buf, 1, &keysym, NULL);

    switch (keysym)
    {
	case XK_Shift_L:
		fprintf(stderr, "Left Shift");
		break;
	case XK_Shift_R:
		fprintf(stderr, "Right Shift");
		break;
	case XK_Control_L:
		fprintf(stderr, "Left Control");
	 	break;
	case XK_Control_R:
		fprintf(stderr, "Right Control");
		break;
	case XK_Caps_Lock:
		fprintf(stderr, "Caps Lock");
		break;
	case XK_Up:
		ggl->ggl_status.Xrot += ggl->ggl_status.rot_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case XK_Down:
		ggl->ggl_status.Xrot -= ggl->ggl_status.rot_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case XK_Left:
		ggl->ggl_status.Zrot += ggl->ggl_status.rot_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case XK_Right:
		ggl->ggl_status.Zrot -= ggl->ggl_status.rot_step;
		ggl_Draw((XtPointer)ggl);
		break;

	default:
		regularKey(ggl, buf);
    }
}

static void
regularKey(gglWidget *ggl, char *buf)
{
    switch(buf[0])
    {
	case 'h':
		ggl->ggl_status.Xtran += ggl->ggl_status.tran_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'l':
		ggl->ggl_status.Xtran -= ggl->ggl_status.tran_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'j':
		ggl->ggl_status.Ytran += ggl->ggl_status.tran_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'k':
		ggl->ggl_status.Ytran -= ggl->ggl_status.tran_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'i':
		ggl->ggl_status.Ztran += ggl->ggl_status.tran_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'o':
		ggl->ggl_status.Ztran -= ggl->ggl_status.tran_step;
		ggl_Draw((XtPointer)ggl);
		break;
	/* scale */                
	case 'a':
		ggl->ggl_status.Xscal += ggl->ggl_status.scal_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 's':
		ggl->ggl_status.Xscal -= ggl->ggl_status.scal_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'd':
		ggl->ggl_status.Yscal += ggl->ggl_status.scal_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'f':
		ggl->ggl_status.Yscal -= ggl->ggl_status.scal_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'w':
		ggl->ggl_status.Zscal += ggl->ggl_status.scal_step;
		ggl_Draw((XtPointer)ggl);
		break;
	case 'e':
		ggl->ggl_status.Zscal -= ggl->ggl_status.scal_step;
		ggl_Draw((XtPointer)ggl);
		break;
	default:
		/* fprintf(stderr, "Key=(%c) \n", c); */
		break;
    }
}

static void
ggl_ButtonPress(gglWidget *ggl, XEvent *event)
{
    if(event->xbutton.button == 2) {
	ggl->ggl_topo.plane.enable = 0;
	ggl_Draw((XtPointer)ggl);
    }
    else if(event->xbutton.button == 3) {
	ggl->ggl_topo.plane.enable = 1;
	ggl->ggl_topo.plane.height = 2.2 - 2.2 * (event->xbutton.y/360.0);
	ggl_Draw((XtPointer)ggl);
    }
    ggl->ggl_status.button.button = event->xbutton.button;
    ggl->ggl_status.button.down = 1;
    ggl->ggl_status.button.X = event->xbutton.x;
    ggl->ggl_status.button.Y = event->xbutton.y;
}

static void
ggl_ButtonRelease(gglWidget *ggl, XEvent *event)
{
    if(event->xbutton.button == 1 && ggl->ggl_status.button.down == 1
		&& ggl->ggl_status.button.button == 1)
    {
	ggl->ggl_status.Xtran += (float)(event->xbutton.x -
					ggl->ggl_status.button.X)/
					ggl->ggl_status.button.scalerate;
	ggl->ggl_status.Ytran -= (float)(event->xbutton.y -
					ggl->ggl_status.button.Y)/
					ggl->ggl_status.button.scalerate;
	ggl_Draw((XtPointer)ggl);

	ggl->ggl_status.button.down = 0;
	ggl->ggl_status.button.X = 0;
	ggl->ggl_status.button.Y = 0;                
    }    
}
    
/* create widget for openGL under the parent widget
 */
gglWidget *
ggl_CreateOpenGLWidget(Widget parent)
{
/*
    int dblBuf[] = {
        GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 16,
        GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
        None
    };

    int *snglBuf = &dblBuf[1];
*/    
    Display *dpy;
/*    GLboolean    doubleBuffer = GL_TRUE; */
    int doubleBuffer = 1;
    XVisualInfo *vi;
    Arg		args[6];
    gglWidget *ggl = NULL;
    int n;

    /* find an OpenGL-capable RGB visual with depth buffer */
    dpy = XtDisplay(parent); 
/*    vi = glXChooseVisual(ggl->ggl_status.dpy,
		DefaultScreen(ggl->ggl_status.dpy), dblBuf); */
    vi = select_truecolor(dpy, XDefaultScreen(dpy), &doubleBuffer);   

    if(vi == NULL)
    {
/*    	vi = glXChooseVisual(dpy, DefaultScreen(ggl->ggl_status.dpy), snglBuf);

	if (vi == NULL){
	    printf("no RGB visual with depth buffer \n");
	    if(ggl) free((void *)ggl);
            return NULL;
        }
    	doubleBuffer = GL_FALSE;
*/        
        printf("no RGB visual with depth buffer \n");
        return NULL;
    }
    ggl = (gglWidget *)malloc(sizeof(gglWidget));
    ggl_InitialzieOpenGL(ggl);
    ggl->ggl_status.dpy = dpy;
    ggl->ggl_status.parent = parent;
/*    ggl->ggl_status.dpy = XtDisplay(parent); */
    
    /* create an OpenGL rendering context */
    ggl->ggl_status.cx = glXCreateContext(ggl->ggl_status.dpy, vi,
		/* no display list sharing */ None,
		/* favor direct */ GL_TRUE);
    if(ggl->ggl_status.cx == NULL)
    {
    	printf("could not create rendering context\n");
        if(ggl)free((void *)ggl);
        return NULL;
    }
    /* create an X colormap since probably not using default visual */

    XtAddEventHandler(parent, StructureNotifyMask, False,
                      ggl_MapStateChangedCallback, NULL);
                      
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], GLwNvisualInfo, vi); n++;

    ggl->ggl_status.glxarea = XtCreateManagedWidget("glxarea",
				glwDrawingAreaWidgetClass, parent, args, n);

    XtAddCallback(ggl->ggl_status.glxarea, XmNexposeCallback,
        (XtCallbackProc)ggl_DrawCallback, (XtPointer)ggl);

    XtAddCallback(ggl->ggl_status.glxarea, XmNresizeCallback,
		ggl_ResizeCallback, (XtPointer)ggl);

    XtAddCallback(ggl->ggl_status.glxarea, XtNdestroyCallback,
		ggl_DestroyCallback, (XtPointer)ggl);
/*
    XtAddCallback(ggl->ggl_status.glxarea, XmNinputCallback, ggl_input, NULL);
*/
    
    XtAddEventHandler(ggl->ggl_status.glxarea,
		    PointerMotionMask | ButtonPressMask | KeyPressMask |
		    KeyReleaseMask|ButtonReleaseMask,
		    False, ggl_InputCallback, (XtPointer)ggl);
    
    /* set up application's window layout */
    XtRealizeWidget(ggl->ggl_status.glxarea); 
    ggl->ggl_status.win = XtWindow(ggl->ggl_status.glxarea);                                  /* For OpenGL */

    /*
     * Once widget is realized (ie, associated with a created X window),
     * we can bind the OpenGL rendering context to the window.
     */
    glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
			ggl->ggl_status.cx);

    ggl->ggl_status.made_current = GL_TRUE;
    /* setup OpenGL state */
    ggl_defaultGL(ggl); 
    ggl_smpDrawSurface(ggl); 

    if(ggl->ggl_status.glxarea)
    {
        glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
		ggl->ggl_status.cx);
        ggl->ggl_topo.drawList = glGenLists(1);
        glNewList(ggl->ggl_topo.drawList, GL_COMPILE);
        ggl_DrawOrdinate(ggl);
        glEndList();
    }    

    return ggl;    
}

void
show_glxvisinfo(Display *dpy, int screen_no)
{
    XVisualInfo viproto, *vip;
    int nvi, i;
    XImage *ximg;

    int glvis, depth, level, dir_true, dbf, stereo, n_aux;
    int rdepth, gdepth, bdepth, adepth, zdepth, stencil;
    int radepth, gadepth, badepth, aadepth;

    viproto.screen = screen_no;
    vip = XGetVisualInfo(dpy, VisualScreenMask, &viproto, &nvi);
    for (i = 0; i < nvi; ++i)
    {
	ximg = XCreateImage(dpy, vip[i].visual, vip[i].depth, ZPixmap, 0,
			NULL, 16, 1, 32, 0);

	printf("\tXImage depth = %d\n", ximg->bytes_per_line * 8 / 16);
	XDestroyImage(ximg);

	switch (vip[i].class) {
	    case PseudoColor: printf("\tPseudoColor\n"); break;
	    case DirectColor: printf("\tDirectColor\n"); break;
	    case GrayScale:   printf("\tGrayScale\n");   break;
	    case StaticColor: printf("\tStaticColor\n"); break;
	    case TrueColor:   printf("\tTrueColor\n");   break;
	    case StaticGray:  printf("\tStaticGray\n");  break;
	};
	printf("\tred_mask, green_mask, blue_mask: 0x%x, 0x%x, 0x%x\n",
	   (int)vip[i].red_mask,
	   (int)vip[i].green_mask,
	   (int)vip[i].blue_mask);
	printf("\tColormap size = %d\n", vip[i].colormap_size);
	printf("\tbits_per_rgb = %d\n", vip[i].bits_per_rgb);

	glXGetConfig(dpy, &vip[i], GLX_USE_GL, &glvis);
	if (glvis == 0) {
	    printf("This visual is not a GLX visual.\n");
	    continue;
	}
	glXGetConfig(dpy, &vip[i], GLX_BUFFER_SIZE,      &depth);
	glXGetConfig(dpy, &vip[i], GLX_LEVEL,            &level);
	glXGetConfig(dpy, &vip[i], GLX_RGBA,             &dir_true);
	glXGetConfig(dpy, &vip[i], GLX_DOUBLEBUFFER,     &dbf);
	glXGetConfig(dpy, &vip[i], GLX_STEREO,           &stereo);
	glXGetConfig(dpy, &vip[i], GLX_AUX_BUFFERS,      &n_aux);
	glXGetConfig(dpy, &vip[i], GLX_RED_SIZE,         &rdepth);
	glXGetConfig(dpy, &vip[i], GLX_GREEN_SIZE,       &gdepth);
	glXGetConfig(dpy, &vip[i], GLX_BLUE_SIZE,        &bdepth);
	glXGetConfig(dpy, &vip[i], GLX_ALPHA_SIZE,       &adepth);
	glXGetConfig(dpy, &vip[i], GLX_DEPTH_SIZE,       &zdepth);
	glXGetConfig(dpy, &vip[i], GLX_STENCIL_SIZE,     &stencil);
	glXGetConfig(dpy, &vip[i], GLX_ACCUM_RED_SIZE,   &radepth);
	glXGetConfig(dpy, &vip[i], GLX_ACCUM_GREEN_SIZE, &gadepth);
	glXGetConfig(dpy, &vip[i], GLX_ACCUM_BLUE_SIZE,  &badepth);
	glXGetConfig(dpy, &vip[i], GLX_ACCUM_ALPHA_SIZE, &aadepth);

	printf("GLX Information:\n");
	printf("\tBuffer size (in bit) = %d", depth);
	printf(" Level = %d\n", level);

	if (dir_true) printf("\tGL is in RGBA mode.\n");
	else printf("\tGL is not in RGBA mode.\n");

	if (stereo) printf("\tStereo is avilable.\n");
	if (dbf) printf("\tDouble Buffer\n");
	printf("\thas %d AUX buffers.\n", n_aux);
	printf("\tsize of red, green, blue, alpha=(%d %d %d %d)\n",
		rdepth, gdepth, bdepth, adepth);
	printf("\tDepth of Z Buffer=%d\n", zdepth);
	printf("\tSTENCIL SIZE=%d\n", stencil);
	printf("\tACCUM BUFFER SIZE(r,g,b,a)=(%d %d %d %d)\n",
		radepth, gadepth, badepth, aadepth);
    }
}

/* Visual
 */
XVisualInfo *
select_truecolor(Display *dpy, int screen_no, int *pisdb)
{
    int i, glvis, depth, dir_true, zdepth;
    int nvi, dbf, md, id, isdb=0;
    XVisualInfo viproto;        /* fill in for getting info */
    XVisualInfo *vip;           /* retured info list */

    md = 0;
    id = 0;

    /* Visual List */
    viproto.screen = screen_no;
    vip = XGetVisualInfo(dpy, VisualScreenMask, &viproto, &nvi);

    if (vip == NULL) return(NULL);

    if (*pisdb)
    {
	for (i = 0; i < nvi; ++i)
	{
	    glXGetConfig(dpy, &vip[i], GLX_USE_GL, &glvis);
	    if (glvis)
	    {
		glXGetConfig(dpy, &vip[i], GLX_BUFFER_SIZE, &depth);
		glXGetConfig(dpy, &vip[i], GLX_RGBA,        &dir_true);
		glXGetConfig(dpy, &vip[i], GLX_DOUBLEBUFFER,&dbf);
		glXGetConfig(dpy, &vip[i], GLX_DEPTH_SIZE,  &zdepth);

		if (dir_true && zdepth > 12 && depth > md && dbf) {
		    md = depth;
		    id = i;
		}
	    }
	}
	if (md > 0) {
	    return(&vip[id]);
	}
    }
    for (i = 0; i < nvi; ++i)
    {
	glXGetConfig(dpy, &vip[i], GLX_USE_GL, &glvis);
	if (glvis)
	{
	    glXGetConfig(dpy, &vip[i], GLX_BUFFER_SIZE, &depth);
	    glXGetConfig(dpy, &vip[i], GLX_RGBA,        &dir_true);
	    glXGetConfig(dpy, &vip[i], GLX_DOUBLEBUFFER,&dbf);
	    glXGetConfig(dpy, &vip[i], GLX_DEPTH_SIZE,  &zdepth);
	    if (dir_true && zdepth >= 12 && depth > md)
	    {
		md = depth;
		id = i;
		isdb = dbf;
	    }
	}
    }
    *pisdb = 0;
    if(md > 0)
    {
	if (isdb != 0) *pisdb = 1;
	return(&vip[id]);
    }
    return(NULL);
}


XVisualInfo *
select_24bitgl(Display *dpy, int screen_no, int *pisdb)
{
    int i, glvis, depth, dir_true, zdepth;
    int nvi, dbf, md, id, isdb=0;
    XVisualInfo viproto;        /* fill in for getting info */
    XVisualInfo *vip;           /* retured info list */

    md = 23;
    id = 0;
    /* Get Visual List */
    viproto.screen = screen_no;
    vip = XGetVisualInfo(dpy, VisualScreenMask, &viproto, &nvi);

    if (vip == NULL) return(NULL);

    for (i = 0; i < nvi; ++i)
    {
	glXGetConfig(dpy, &vip[i], GLX_USE_GL, &glvis);
	if (glvis)
	{
	    glXGetConfig(dpy, &vip[i], GLX_BUFFER_SIZE, &depth);
	    glXGetConfig(dpy, &vip[i], GLX_RGBA,        &dir_true);
	    glXGetConfig(dpy, &vip[i], GLX_DOUBLEBUFFER,&dbf);
	    glXGetConfig(dpy, &vip[i], GLX_DEPTH_SIZE,  &zdepth);

	    if (dir_true && zdepth > 1 && depth > md) {
		md = depth;
		id = i;
		isdb = dbf;
	    }
	}
    }
    *pisdb = 0;
    if(md > 0)
    {
	if(isdb != 0) *pisdb = 1;
	return(&vip[id]);
    }
    return(NULL);
}

void
ggl_SetXYScale(gglWidget *ggl, double x_min, double x_max,
		double y_min, double y_max)
{
    double dif;
    CurrentFK *d;
    if(ggl == NULL) return;

    d = &ggl->data;
    d->x_max = x_max;
    d->x_min = x_min;
    d->y_max = y_max;
    d->y_min = y_min;
    dif = d->x_max - d->x_min;
    d->unscale_x = (dif != 0.) ? (d->ox_max - d->ox_min)/dif : 1.;
    dif = d->y_max - d->y_min;
    d->unscale_y = (dif != 0.) ? (d->oy_max - d->oy_min)/dif : 1.;
    d->scale_x = (d->unscale_x != 0.) ? 1./d->unscale_x : 1.;
    d->scale_y = (d->unscale_y != 0.) ? 1./d->unscale_y : 1.;
}

void
ggl_SetScale(gglWidget *ggl, double x_min, double x_max, double y_min,
		double y_max, float z_min, float z_max)
{
    double dif;
    CurrentFK *d;
    if(ggl == NULL) return;

    d = &ggl->data;
    d->x_max = x_max;
    d->x_min = x_min;
    d->y_max = y_max;
    d->y_min = y_min;
    d->z_max = z_max;
    d->z_min = z_min;
    dif = d->x_max - d->x_min;
    d->unscale_x = (dif != 0.) ? (d->ox_max - d->ox_min)/dif : 1.;
    dif = d->y_max - d->y_min;
    d->unscale_y = (dif != 0.) ? (d->oy_max - d->oy_min)/dif : 1.;
    dif = d->z_max - d->z_min;
    d->unscale_z = (dif != 0.) ? (d->oz_max - d->oz_min)/dif : 1.;
    d->scale_x = (d->unscale_x != 0.) ? 1./d->unscale_x : 1.;
    d->scale_y = (d->unscale_y != 0.) ? 1./d->unscale_y : 1.;
    d->scale_z = (d->unscale_z != 0.) ? 1./d->unscale_z : 1.;
}

/* added these defines for speed */
#define unscale_x(d,x) (d->ox_min + (x - d->x_min)*d->unscale_x)
#define unscale_y(d,y) (d->oy_min + (y - d->y_min)*d->unscale_y)
#define unscale_z(d,z) (d->oz_min + (z - d->z_min)*d->unscale_z)

void
ggl_ShowOneFK(gglWidget *ggl, int nx, double *x, int ny, double *y, float *z,
		int local_color)
{
    int i, n;
    CurrentFK *d = &ggl->data;

    if(!ggl) return;

    if(d->z == NULL || nx*ny != d->nx*d->ny) {
	Free(d->z);
	if((d->z = (float *)malloc(nx*ny*sizeof(float))) == NULL) {
	    fprintf(stderr, "ggl_ShowOneFK: malloc error.\n");
	    return;
	}
    }

    if(d->x == NULL || nx != d->nx) {
	Free(d->x);
	if((d->x = (float *)malloc(nx*sizeof(float))) == NULL) {
	    fprintf(stderr, "ggl_ShowOneFK: malloc error.\n");
	    return;
	}
    }

    if(d->y == NULL || ny != d->ny) {
	Free(d->y);
	if((d->y = (float *)malloc(ny*sizeof(float))) == NULL) {
	    fprintf(stderr, "ggl_ShowOneFK: malloc error.\n");
	    return;
	}
    }

    for(i = 0; i < nx; i++)
    {
	d->x[i] = unscale_x(d, x[i]);
    }
    for(i = 0; i < ny; i++) {
	d->y[i] = unscale_y(d, y[i]);
    }

    n = nx * ny;
    for(i = 0; i < n; i++) {
	d->z[i] = unscale_z(d, z[i]);
    }
    d->nx = nx;
    d->ny = ny;
    d->computeAll = 0;    
    /*    compute the contour points
     */
    ggl->contours.slowness_min = d->x_min;
    ggl->contours.slowness_max = d->x_max;
    ggl->contours.n_slowness = d->nx;
    ggl->ggl_status.local_color = local_color;

    ggl_Redraw(ggl);
    return;
}

void
ggl_Redraw(gglWidget *ggl)
{
    if(ggl == NULL) return;

    if(ggl->ggl_status.glxarea)
    {
        glXMakeCurrent(ggl->ggl_status.dpy, XtWindow(ggl->ggl_status.glxarea),
			ggl->ggl_status.cx);
        glNewList(ggl->ggl_topo.drawList, GL_COMPILE);

        ggl_DrawOrdinate(ggl);

        if(ggl->data.z) {
            ggl_DrawSurface(ggl);
	}
        glEndList();
    }    
    ggl_Draw((XtPointer)ggl);
}

#endif /* HAVE_OPENGL */
