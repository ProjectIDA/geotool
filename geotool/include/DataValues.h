#ifndef _DATA_VALUES_H
#define _DATA_VALUES_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"

class Table;

/**
 *  @ingroup libgx
 */
class DataValues : public FormDialog
{
    public:
	DataValues(const string &, Component *, WaveformPlot *);
	~DataValues(void);

	void actionPerformed(ActionEvent *action_event);
	void setVisible(bool visible);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

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
