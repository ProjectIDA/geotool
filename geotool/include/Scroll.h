#ifndef _SCROLL_H
#define _SCROLL_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"

/**
 *  @ingroup libgx
 */
class Scroll : public FormDialog
{
    public:
	Scroll(const string &, Component *, WaveformPlot *);
	~Scroll(void);

	void actionPerformed(ActionEvent *action_event);
	void setVisible(bool visible);

    protected:
	RowColumn *controls;
	Button *close_button;
	Separator *sep;
	Scale *speed_scale;
	RadioBox *rb;
	Toggle *fb_toggle, *b_toggle, *s_toggle, *f_toggle, *ff_toggle;

 	WaveformPlot *wp;
	int action;
	unsigned long interval;
	char direction[12];

	void createInterface(void);

	static void doScroll(XtPointer client_data, XtIntervalId id);
};

#endif
