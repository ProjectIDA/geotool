#ifndef _MOTION_PLOT_H
#define _MOTION_PLOT_H

#include "motif++/MotifDecs.h"
#include "motif++/FormDialog.h"
#include "DataReceiver.h"
#include "gobject++/gvector.h"
#include "ParticleMotionClass.h"
#include "widget/DialClass.h"
#include "widget/AxesClass.h"

/** @defgroup libgpm plugin Particle Motion
 */

class DataSource;
class WaveformPlot;
class Iaspei;
class BasicMap;

namespace libgpm {

/** MotionPlot window.
 *  @ingroup libgpm
 */
class MotionPlot : public FormDialog, DataReceiver
{
    public:
	MotionPlot(const char *, Component *, DataSource *ds);
	~MotionPlot(void);

	virtual void setDataSource(DataSource *ds);
	void setVisible(bool visible);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	RowColumn *controls, *dial_rc, *rc;
	Toggle *long_arrow_toggle, *map_toggle;
	Button *close_button, *save_button, *new_pm_button, *help_button;
	Choice *input_choice;
	Separator *sep;
	Label *label;
	Form *form;
	Label *W_label, *E_label, *N_label, *S_label;
	DialClass *dial;
	Label *azimuth_label, *incidence_label;
	Button *origin_button, *maximize_button;
	Choice *phase_choice;
	TextField *azimuth_text, *incidence_text, *distance_text;
	ScrollBar *scrollbar;
	ParticleMotionClass *pm;

	WaveformPlot *wp;
	bool remove_cursor;
	gvector<MotionPlot *> windows;
 	Iaspei *iaspei;
	BasicMap *map;

	bool warned;
	int id, p_npts, s_npts;
	float p_tt[500], p_dist[500], p_ray_p[500];
	float s_tt[500], s_dist[500], s_ray_p[500];
	double max_azimuth, max_incidence;


	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
//	void print(void);
	void dialAction(DialCallbackStruct *s);
	void scrollAction(XmScrollBarCallbackStruct *s);
	void displayDataCursors(void);
	void removeDataCursors(void);
	void setArrowLength(void);
	void newWindow(void);
	void getRayParameters(double depth);
	void computeDistance(double incidence);
	void setAzimuth(void);
	void setIncidence(void);
	void setOrigin(void);
	void drawOnMap(int reason);
	void doubleLineCB(AxesCursorCallbackStruct *c);
	void rotateMax(void);
	void saveAzInc(void);
	void unrotate(int npts, float **e, float **n, float **z,
		double alpha, double beta, double gamma);
	void getRectilinearity(int npts, float *e, double e_mean, float *n,
		double n_mean, float *z, double z_mean);

    private:

};

} // namespace libgpm

#endif
