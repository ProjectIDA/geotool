#ifndef _EXAMPLE_4_
#define _EXAMPLE_4_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"
#include "widget/DialClass.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"

/* Change the following defines as desired. Make sure that they do not conflict
 * with another plug-in. PLUGIN_STRING must be "PLUGIN_NAME".
 */
#define PLUGIN_NAME		Example4
#define PLUGIN_STRING		"Example4"
#define PLUGIN_CLASS		Example4Plugin
#define PLUGIN_DESCRIPTION	"Rotation Example Plug-in"
#define PLUGIN_BUTTON_NAME	"Example 4 (Rotation)..."

class PLUGIN_NAME : public FormDialog, public DataReceiver
{
    public:
	PLUGIN_NAME(const char *, Component *, DataSource *);
	~PLUGIN_NAME(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	RowColumn *controls, *rc;
	Button *close_button, *rotate_button, *unrotate_button, *help_button;
	Separator *sep;
	Label *azimuth_label, *incidence_label;
	TextField *azimuth_text, *incidence_text;

	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	bool rotateWaveforms(double azimuth, Waveform *e_w, Waveform *n_w);
	bool rotateWaveforms(double azimuth, double incidence,
			Waveform *e_w, Waveform *n_w, Waveform *z_w);
	void rotate(void);
	void unrotate(void);
	bool getHang(GTimeSeries *ts, double *hang, double *vang);
	int checkComponents(int *num_groups, int *num_cmpts,
			Waveform **wvec);
};

#endif
