#ifndef _ARRIVAL_KEY_TABLE_H
#define _ARRIVAL_KEY_TABLE_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"
#include "widget/Table.h"

class ArrivalKeyTable : public FormDialog
{
    public:
	ArrivalKeyTable(const string &name, Component *parent,WaveformPlot *wp);
	~ArrivalKeyTable(void);

	void loadTable(void);
	void setVisible(bool visible);

    protected:
	WaveformPlot	*wplot;
	Table		*table;

	RowColumn	*rc;
	Button		*close_button, *apply_button, *remove_button;
	Separator	*sep;

	Form		*form;
	Button		*key_plus_button, *key_minus_button;

	void actionPerformed(ActionEvent *a);
	void apply(void);
	void remove(void);
	void buttonsSensitive(void);

    private:
};

#endif
