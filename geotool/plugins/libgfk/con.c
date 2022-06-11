/** \file con.c
 *  \brief Unfinished 3-D contouring routines.
 *  \author Ivan Henson
 */
#include "config.h"
#include "libgplot.h"
#include "libdrawx.h"

#ifdef HAVE_OPENGL

#include "ggl.h"


void ggl_getContours(gglLineProjection *ggl, float *fk)
{
	DrawStruct d;
	int	i, contour_label_size = 1;
	Matrx	m;
	Boolean	auto_ci = True;
	float	ci, c_min = 0., c_max = 0.;
	double	xmin, xmax, ymin, ymax, d_slowness;
	LabelArray *l;

	/* Set the unscaled limits. Must fit in a short int. Make it large
	 * and then scale to (-1., 1.) later.
	 */
	SetDrawArea(&d, -30000, -30000, 30000, 30000);

	xmin = ggl->slowness_min;
	xmax = ggl->slowness_max;
	ymin = ggl->slowness_min;
	ymax = ggl->slowness_max;

	/* all x,y values between xmin,xmax and ymin,ymax will be scaled
	 * to -30000,30000 (short ints in ggl->segs)
	 */
	SetScale(&d, xmin, ymin, xmax, ymax);

	if((m.x = (double *)malloc(ggl->n_slowness*sizeof(double))) == NULL) {
	    fprintf(stderr, "ggl_getContours: malloc error.\n");
	    return;
	}

	d_slowness = (ggl->slowness_max - ggl->slowness_min)/
			(double)(ggl->n_slowness-1);

	for(i = 0; i < ggl->n_slowness; i++) {
	    m.x[i] = ggl->slowness_min + i*d_slowness;
	}
	m.y = m.x;
	m.z = fk;

	if(ggl->s.segs != NULL) free(ggl->s.segs);
	if(ggl->t.segs != NULL) free(ggl->t.segs);
	if(ggl->l.ls != NULL) free(ggl->l.ls);
	ggl->s.size_segs = 0;
	ggl->s.nsegs = 0;
	ggl->s.segs = NULL;
	ggl->t.size_segs = 0;
	ggl->t.nsegs = 0;
	ggl->t.segs = NULL;
	ggl->l.size_ls = 0;
	ggl->l.n_ls = 0;
	ggl->l.ls = NULL;

	m.x1 = xmin;
	m.x2 = xmax;
	m.y1 = xmin;
	m.y2 = xmax;
	m.nx = ggl->n_slowness;
	m.ny = ggl->n_slowness;
	m.imin = m.jmin = 0;
	m.imax = m.jmax = 0;

	d.s = &ggl->s;
	d.l = &ggl->l;

	condata(&d, (float)contour_label_size, &m, auto_ci, &ci, c_min, c_max);

	free(m.x);

	d.s = &ggl->t;
        l = &ggl->l;

	for(i = 0; i < l->n_ls; i++)
	{
	    mapalf(&d, l->ls[i].x, l->ls[i].y, l->ls[i].size, l->ls[i].angle,
			0, l->ls[i].label);
	}
}


/* to draw the contours */

/*
	drawSegArray(&ggl->s);
	drawSegArray(&ggl->t);
*/

/*
static void
drawSegArray(SegArray *s)
{
	int i;
	XSegment *segs;
	double scale = 1./30000.;
	float x, y;

	glBegin(GL_LINES);

	segs = s->segs;
	for(i = 0; i < s->nsegs; i++)
	{
	    x = segs[i].x1*scale;
	    y = segs[i].y1*scale;
	    glVertex3f(x, y, 0.01);

	    x = segs[i].x2*scale;
	    y = segs[i].y2*scale;
	    glVertex3f(x, y, 0.01);
	}
	glEnd();
}
*/


void ggl_drawContours(gglWidget *ggl,int recal)
{
/*
    if(!ggl->current_fk.realfk)return;
    if(recal || (!ggl->contours.valid)){
        ggl->contours.slowness_min = ggl->current_fk.x_min;
        ggl->contours.slowness_max = ggl->current_fk.x_max;
        ggl_getContours(&ggl->contours,ggl->current_fk.realfk);
      	ggl->contours.valid = 1;
    }
    drawSegArray(&ggl->contours.s);
    drawSegArray(&ggl->contours.t);
*/
}

#endif /* HAVE_OPENGL */ 
