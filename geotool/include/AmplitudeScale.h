#ifndef _AMPLITUDE_SCALE_H
#define _AMPLITUDE_SCALE_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "motif++/Toggle.h"
#include "motif++/TextField.h"

#define APPLY_LAYOUT		0
#define APPLY_SCALE		1
#define SET_TYPE_INDEPENDENT	2
#define SET_TYPE_UNIFORM	3

/** A FormDialog subclass for setting the waveform scale. This interface allows
 *  the waveform height and the separation between waveforms to be controlled.
 *  The height and separation can be set for all or only selected waveforms and
 *  the scale can be computed based on either the entire waveform or only the
 *  visible portion of the waveform.
 * 
 *  AmplitudeScale class callbacks:
 *     - XtNactivateCallback is called with the following calldata:
 *		- APPLY_LAYOUT the Apply Layout button was activated.
 *		- APPLY_SCALE the Apply Scale button was activated.
 *		- SET_TYPE_INDEPENDENT the Independent Toggle was selected.
 *		- SET_TYPE_UNIFORM the Uniform Toggle was selected.
 *  \image html AmplitudeScale.gif "The Amplitude Scale Window"
 *  \image latex AmplitudeScale.eps "The Amplitude Scale Window" width=3in
 *  @ingroup libgx
 */
class AmplitudeScale : public FormDialog
{
    public:
	AmplitudeScale(const string &name, Component *parent);
	~AmplitudeScale(void);

	void actionPerformed(ActionEvent *action_event);

	/** Return the Uniform Toggle Component. */
	Toggle *uniformToggle(void) { return uniform_toggle; }
	/** Return the Independent Toggle Component. */
	Toggle *independentToggle(void) { return independent_toggle; }
	/** Get the Initial Data Height text. */
	char *getDataHeight(void) {return data_height_text->getString();}
	/** Get the Initial Data Separation text. */
	char *getDataSeparation(void){return data_separation_text->getString();}
	/** Get the Current Data Height text. */
	char *getScaleDataHeight(void) {
	    return scale_data_height_text->getString();
	}
	/** Set the Initial Data Height text. */
	void setDataHeight(char *text) {data_height_text->setString(text);}
	/** Set the Initial Data Separation text. */
	void setDataSeparation(char *text) {
	    data_separation_text->setString(text);
	}
	/** Set the state of the All Toggle. */
	bool getAllState(void) { return all_toggle->state(); }
	/** Set the state of the Visible Toggle. */
	bool getVisibleState(void) { return visible_toggle->state(); }
	/** Set the state of the Uniform Toggle. */
	bool getUniformState(void) { return uniform_toggle->state(); }
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:

	Label *initial_layout_label, *data_height_label1;
	Label *data_separation_label, *set_current_scale_label;
	Label *waveforms_label, *time_period_label, *set_type_label;
	Form *form1, *form2, *form3;
	Separator *sep1, *sep2, *sep3;
	TextField *data_height_text, *data_separation_text;
	TextField *scale_data_height_text;
	Button *apply_layout_button, *apply_scale_button, *close_button;
	Button *help_button;
	RadioBox *scale_rb, *visible_rb, *type_rb;
	Toggle *all_toggle, *selected_toggle, *visible_toggle, *entire_toggle;
	Toggle *independent_toggle, *uniform_toggle;
	RowColumn *controls;

	void createInterface(void);

    private:
};

#endif
