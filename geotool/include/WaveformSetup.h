#ifndef _WAVEFORM_SETUP_H
#define _WAVEFORM_SETUP_H

#include "motif++/FormDialog.h"
#include "motif++/MotifClasses.h"

/**
 *  @ingroup libgx
 */
class WaveformSetup : public FormDialog
{
    public:
	WaveformSetup(const string &name, Component *parent,
		ActionListener *listener, double join_time_limit,
		double overlap_limit, bool apply_calib);
	~WaveformSetup(void);

	void actionPerformed(ActionEvent *action_event);

	double joinTimeLimit(void) { return join_time_limit; }
	double overlapLimit(void) { return overlap_limit; }
	double segmentLength(void) { return segment_length; }
	bool applyCalib(void) { return calib_toggle->state(); }
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:

	Label *segment_label, *gap_label, *length_label, *overlap_label;
	Label *calib_label;
	RowColumn *rc, *controls;
	TextField *gap_text, *overlap_text, *length_text;
	Separator *sep1, *sep2;
	RadioBox *rb;
	Toggle *calib_toggle, *counts_toggle;
	Button *apply_button, *cancel_button;

	double join_time_limit;
	double overlap_limit;
	double segment_length;
	bool apply_calib;
 
	void createInterface(void);
	void cancel(void);
	void apply(void);
	void setButtonsSensitive(void);

    private:
};

#endif
