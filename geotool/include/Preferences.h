#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"

class ArrivalKeyTable;
class KeyActions;

/**
 *  @ingroup libgx
 */
class Preferences : public FormDialog
{
    public:
	Preferences(const string &, Component *, WaveformPlot *);
	~Preferences(void);

	void actionPerformed(ActionEvent *action_event);
	virtual void setVisible(bool visible);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:
	WaveformPlot *wp;
	ArrivalKeyTable *arrival_keys;
	KeyActions *key_actions;

	RowColumn *controls, *rc, *rc1, *rc2, *rc3, *button_rc;
	Button *arrival_keys_button, *arrival_param_button, *fonts_button;
	Button *key_action_button;
	Button *amp_param_button;
	Button *cancel_button, *help_button;
	Separator *sep1, *sep2, *sep3;
	Label *label, *waveform_label, *arrival_label, *scrollbar_label;
	RadioBox *rb1, *rb2, *rb3;
	Toggle *waveform_on_toggle, *waveform_off_toggle;
	Toggle *arrival_on_toggle, *arrival_off_toggle;
	Toggle *left_toggle, *right_toggle;
	Choice *load_origins;

	void scrollBarLeft(bool left);
};

#endif
