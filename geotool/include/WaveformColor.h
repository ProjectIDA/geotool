#ifndef _WAVEFORM_COLOR_H
#define _WAVEFORM_COLOR_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "ColorTable.h"

/**
 *  @ingroup libgx
 */
class WaveformColor : public FormDialog
{
    public:
	WaveformColor(const string &, Component *, ActionListener *);
	~WaveformColor(void);

	void actionPerformed(ActionEvent *action_event);
	void setColor(const string &color);
	ColorTable *colorTable(void) {
	    if( !color_table ) {
		color_table = new ColorTable("Waveform Color List", this);
	    }
	    return color_table;
	}

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	Label *label;
	RowColumn *controls;
	Button *close_button, *colors_button;
	Separator *sep;
	List *list;
	ColorTable *color_table;

};

#endif
