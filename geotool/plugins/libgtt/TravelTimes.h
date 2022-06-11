#ifndef _TRAVEL_TIMES_H
#define _TRAVEL_TIMES_H

#include "motif++/FormDialog.h"
#include "DataReceiver.h"
#include "motif++/MotifDecs.h"
#include "gobject++/DataSource.h"

extern "C" {
#include "crust.h"
}

/** @defgroup libgtt plugin Travel Times
 */

class WaveformWindow;
class WaveformPlot;
class OridList;

namespace libgtt {

/** Travel Times window.
 *  @ingroup libgtt
 */
class TravelTimes : public FormDialog, public DataReceiver
{
    public:
	TravelTimes(const char *name, Component *parent, DataSource *ds);
	~TravelTimes(void);

	void curves(bool on);
	void setDataSource(DataSource *ds);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	RowColumn *rc, *r1, *r2, *r3, *rc2, *r4, *r5, *rc_select;
	Form *form1, *form2;
	RowColumn *reduced, *depth_rc, *controls;

	Toggle *labels_toggle, *curves_toggle, *scroll_toggle, *reduced_time;
	Toggle *degrees_toggle, *kilometers_toggle;
	Button *select_all_button, *deselect_all_button;
	Button *reduced_apply, *depth_apply, *close_button, *help_button;

	Separator *sep, *sep1, *sep2, *sep3, *sep4;
	Label *lab1, *lab2, *lab3, *lab4, *lab5;
	ScrolledWindow *sw1, *sw2, *sw3, *sw4;

	List *iaspei_list, *jb_list, *crust_list, *regional_list;
	TextField *crust_text, *velocity_text, *depth_text;

	Scale *depth_scale;

	WaveformWindow *ww;
	WaveformPlot *wplot;

	bool save_selected;

	OridList	*orid_list;
	int		num_crusts;
	CrustModel	*crust_models;

	double ln_max, scale_depth_max;

	void actionPerformed(ActionEvent *action_event);
	void selectPhase(void);
	void saveSelectedPhases(int total_selected, char **selected_phases);
	void crustInit(void);
	void loadCrustModels(void);
	void initPhaseSelections(void);
	void selectPhases(List *list);
	void crustSelect(void);
	void depthScale(void);
	void depthApply(void);
	void depthText(void);
	void selectAllPhases(void);
	void deselectAllPhases(void);
	void reducedTime(double reduction_vel);

    private:

};

} // namespace libgtt

#endif
