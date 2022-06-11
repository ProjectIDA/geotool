#ifndef _DIFF_H
#define _DIFF_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"

class Table;

/**
 *  @ingroup libgx
 */
class Diff : public FormDialog
{
    public:
	Diff(const string &, Component *, WaveformPlot *);
	~Diff(void);

	void actionPerformed(ActionEvent *action_event);
	void setVisible(bool visible);

    protected:
	RowColumn *controls;
	Button *close_button;
	Separator *sep;
	Table *table;

 	WaveformPlot *wp;
	bool remove_cursor;

	void createInterface(void);
	void displayDataCursor(void);
	void removeDataCursor(void);
	void list(void);
};

#endif
