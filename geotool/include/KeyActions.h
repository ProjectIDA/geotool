#ifndef _KEY_ACTIONS_H_
#define _KEY_ACTIONS_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"

class KeyActions : public FormDialog
{
    public:
	KeyActions(const string &name, Component *parent);
	~KeyActions(void);

	void loadTable(void);
	void setVisible(bool visible);

    protected:
	Table		*table;

	RowColumn	*rc;
	Button		*close_button, *apply_button, *remove_button;
	Button		*defaults_button;
	Separator	*sep;

	Form		*form;
	Button		*key_plus_button, *key_minus_button;

	void actionPerformed(ActionEvent *a);
	void apply(void);
	void remove(void);
	void buttonsSensitive(void);
	void setDefaults(void);

    private:
};

#endif
