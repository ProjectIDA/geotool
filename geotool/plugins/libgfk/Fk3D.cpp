/** \file Fk3D.cpp
 *  \brief Defines class Fk3D.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>
using namespace std;

#ifdef HAVE_OPENGL

#include "Fk3D.h"
#include "FKParam.h"
#include "FKData.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "ggl.h"
}

using namespace libgfk;

Fk3D::Fk3D(const char *name, Component *parent, FK *fk_src) :
		Frame(name, parent, true, true)
{
    fk = fk_src;
    createInterface(FK_SINGLE_BAND);
}

Fk3D::Fk3D(const char *name, Component *parent, FK *fk_src, FKType fk_type) :
		Frame(name, parent, true, true)
{
    fk = fk_src;
    createInterface(fk_type);
}

void Fk3D::createInterface(FKType fk_type)
{
    type = fk_type;

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this, INFO_LEFT_ONLY);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    view_menu = new Menu("View", menu_bar);
    grid_toggle = new Toggle("Grid", view_menu, this, false);
    normalize_toggle = new Toggle("Normalize Color Scale", view_menu, this,
				false);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    fk_help_button = new Button("3D Help", help_menu, this);

    if(type == FK_SINGLE_BAND) {
	createSingleInterface();
    }
    else {
	createMultiInterface();
    }

    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNprocessingDirection, XmMAX_ON_LEFT); n++;
    XtSetArg(args[n], XmNminimum, 0); n++;
    XtSetArg(args[n], XmNmaximum, 1000); n++;
    XtSetArg(args[n], XmNvalue, 1000); n++;
    XtSetArg(args[n], XmNshowValue, false); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    scale = new Scale("scale", info_area->rightArea(), this, args, n);

    addPlugins("Fk3D", NULL, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(grid_toggle, "Grid");
	tool_bar->add(normalize_toggle, "Normalize Color Scale");
    }
}

void Fk3D::createSingleInterface(void)
{
    setSize(600, 500);
    manage();
    ggl[0] = ggl_CreateOpenGLWidget(frame_form->baseWidget());
}

void Fk3D::createMultiInterface(void)
{
    int n;
    Arg args[20];

    setSize(580, 680);
    manage();

    n = 0;
    XtSetArg(args[n], XmNfractionBase, 2); n++;
    frame_form->setValues(args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("blue")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("white")); n++;
    label1 = new Label("Frequencies 0.6 - 3.0", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNbottomPosition, 1); n++;
    form1 = new Form("form1", frame_form, args, n);

    ggl[0] = ggl_CreateOpenGLWidget(form1->baseWidget());

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("blue")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("white")); n++;
    label2 = new Label("Frequencies 2.0 - 4.0", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label2->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNbottomPosition, 1); n++;
    form2 = new Form("form2", frame_form, args, n);

    ggl[1] = ggl_CreateOpenGLWidget(form2->baseWidget());

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNtopPosition, 1); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("blue")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("white")); n++;
    label3 = new Label("Frequencies 3.0 - 5.0", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label3->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNrightPosition, 1); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    form3 = new Form("form3", frame_form, args, n);

    ggl[2] = ggl_CreateOpenGLWidget(form3->baseWidget());

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNtopPosition, 1); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("blue")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("white")); n++;
    label4 = new Label("Frequencies 4.0 - 6.0", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label4->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_POSITION); n++;
    XtSetArg(args[n], XmNleftPosition, 1); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    form4 = new Form("form4", frame_form, args, n);

    ggl[3] = ggl_CreateOpenGLWidget(form4->baseWidget());
}

Fk3D::~Fk3D(void)
{
}

void Fk3D::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Grid"))
    {
	int n = (type == FK_SINGLE_BAND) ? 1 : 4;
	int set = (int)grid_toggle->state();
	for(int i = 0; i < n; i++) {
	    ggl_SetDrawType(ggl[i], set);
	    ggl_Redraw(ggl[i]);
	}
    }
    else if(!strcmp(cmd, "Normalize Color Scale") || !strcmp(cmd, "scale"))
    {
	draw();
    }
    else if(!strcmp(cmd, "3D Help")) {
	showHelp("FK-3D Help");
    }
}

void Fk3D::set3DScale(int i, double x_min, double x_max, double y_min,
			double y_max, float fk_min, float fk_max)
{
    int n;
    Arg args[3];
    int value=0, min=0, max=0;
    double d;

    n = 0;
    XtSetArg(args[n], XmNvalue, &value); n++;
    XtSetArg(args[n], XmNminimum, &min); n++;
    XtSetArg(args[n], XmNmaximum, &max); n++;
    scale->getValues(args, 3);

    d = (max > min) ? value/(double)(max - min) : 1.;
    if(d < .001) d = .001;
    fk_max = fk_min + d*(fk_max - fk_min);

    if(i >= 0 && i < 4 && ggl[i]) {
	ggl_SetScale(ggl[i], x_min, x_max, y_min, y_max, fk_min, fk_max);
    }
}

void Fk3D::setXYScale(int i, double x_min, double x_max, double y_min,
			double y_max)
{
    if(i >= 0 && i < 4 && ggl[i]) {
	ggl_SetXYScale(ggl[i], x_min, x_max, y_min, y_max);
    }
}

void Fk3D::showOneFK(int i, int nx, double *slow_x, int ny, double *slow_y,
			float *fK)
{
    if(i >= 0 && i < 4 && ggl[i]) {
	int local_color = !normalize_toggle->state();
	ggl_ShowOneFK(ggl[i], nx, slow_x, ny, slow_y, fK, local_color);
    }
}

void Fk3D::setBandLabels(void)
{
    if(type == FK_MULTI_BAND)
    {
	char label[100];

	snprintf(label, sizeof(label), "Frequencies %.2f - %.2f",
			fk->getFMin(0), fk->getFMax(0));
	label1->setLabel(label);
	snprintf(label, sizeof(label), "Frequencies %.2f - %.2f",
			fk->getFMin(1), fk->getFMax(1));
	label2->setLabel(label);
	snprintf(label, sizeof(label), "Frequencies %.2f - %.2f",
			fk->getFMin(2), fk->getFMax(2));
	label3->setLabel(label);
	snprintf(label, sizeof(label), "Frequencies %.2f - %.2f",
			fk->getFMin(3), fk->getFMax(3));
	label4->setLabel(label);
    }
}

void Fk3D::draw(void)
{
    FKParam *p = fk->getFKParam();
    if(!fk->show_single && p->num_fkdata > 0)
    {
	for(int b = 0; b < p->nbands; b++) {
	    Matrx *m = fk->getData(b);
	    set3DScale(b, 0., 0., 0., 0., m->z_min, m->z_max);
	}
	drawFK(1., -1);
    }
    else if(fk->show_single && p->single_fk != NULL)
    {
	drawFK(0., -1);
    }
}

void Fk3D::drawFK(double slow_limit, long band)
{
    FKParam *p = fk->getFKParam();

    if(band < 0) {
	for(int b = 0; b < p->nbands; b++)
	{
	    Matrx *m = fk->getData(b);
	    set3DScale(b, 0., slow_limit, 0., slow_limit, m->z_min, m->z_max);
	    setXYScale(b, m->x[0], m->x[m->nx-1], m->y[0], m->y[m->ny-1]);
	    showOneFK(b, m->nx, m->x, m->ny, m->y, m->z);
	}
    }
    else {
	Matrx *m = fk->getData(band);
	set3DScale(band, 0., slow_limit, 0., slow_limit, m->z_min, m->z_max);
	setXYScale(band, m->x[0], m->x[m->nx-1], m->y[0], m->y[m->ny-1]);
	showOneFK(band, m->nx, m->x, m->ny, m->y, m->z);
    }
    setBandLabels();
}

#endif /* HAVE_OPENGL */
