#ifndef _ADD_RECIPE_H
#define _ADD_RECIPE_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "Beam.h"
#include "Recipes.h"

class Table;

namespace libgbm {

class AddRecipe : public FormDialog
{
    public:
	AddRecipe(const char *, Component *, RecipeType, ActionListener *);
	~AddRecipe(void);

	void list(BeamGroup *bg);
	void setVisible(bool visible);

    protected:
	Label *label1, *label2, *info_label;
	Form *form;
	FileChoice *recipe_dir_choice;
	RowColumn *controls;
	Button *add_button, *cancel_button;
	Separator *sep;
	Table *table;
	RecipeType type;

	void createInterface(void);
	void setRecipeDirChoice(void);
	void actionPerformed(ActionEvent *a);
	bool addRecipe(void);
	void setButtonsSensitive(void);

    private:

};

} // libgbm

#endif
