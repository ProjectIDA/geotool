#ifndef _OUTPUT_H
#define _OUTPUT_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"

class CssFileDialog;

/**
 *  @ingroup libgx
 */
class Output : public FormDialog
{
    public:
	Output(const string &, Component *, WaveformPlot *);
	~Output(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:
	WaveformPlot *wp;
	CssFileDialog *open_prefix_file;

	RowColumn *controls, *rc, *rc2;
	Button *close_button, *apply_button;
	Separator *sep;
	Label *label, *remark_label;
	Button *prefix_button;
	TextField *prefix_text, *remark_text;
	RadioBox *rb1, *rb2, *rb3, *rb4;
	Toggle *append_toggle, *overwrite_toggle, *raw_toggle;
	Toggle *as_displayed_toggle, *all_toggle, *selected_toggle;
	Toggle *css_toggle, *sac_toggle, *ascii_toggle;
	Toggle *tables_toggle;

	void chooseFile(void);
	void apply(void);
};

#endif
