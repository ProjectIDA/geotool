#ifndef _ADD_GROUP_H
#define _ADD_GROUP_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "Beam.h"

class Table;
class TableSource;
class DataSource;

namespace libgbm {

class AddGroup : public FormDialog, DataReceiver
{
    public:
	AddGroup(const char *, Component *, ActionListener *);
	~AddGroup(void);

	void listNetworks();

    protected:
	Label *label, *label2, *label3;
	Form *form, *form2, *form3;
	FileChoice *recipe_dir_choice;
	TextField *group_text;
	RowColumn *controls;
	Button *add_button, *cancel_button;
	Separator *sep;
	Table *net_table, *group_table;
	TableSource *table_source;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void setRecipeDirChoice(void);
	void selectNetwork(void);
	bool addGroup(void);

    private:

};

} // namespace libgbm

#endif
