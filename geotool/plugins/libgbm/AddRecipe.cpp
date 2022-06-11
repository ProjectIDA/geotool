/** \file AddRecipe.cpp
 *  \brief Defines class AddRecipe
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>
#include <math.h>

using namespace std;

#include "AddRecipe.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"

extern "C" {
#include "libstring.h"
}

using namespace libgbm;

AddRecipe::AddRecipe(const char *name, Component *parent,RecipeType recipe_type,
		ActionListener *listener) : FormDialog(name, parent, false)
{
    type = recipe_type;
    createInterface();
    setRecipeDirChoice();
    enableCallbackType(XmNactivateCallback);
    addActionListener(listener);
    Application::addPropertyListener(this);
}

void AddRecipe::createInterface()
{
    int n;
    Arg args[25];

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    if(type == ORIGIN_RECIPES) {
	label1 = new Label("Add Origin Beam Recipe", this, args, n);
    }
    else {
	label1 = new Label("Add Detection Beam Recipe", this, args, n);
    }

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    label2 = new Label("Recipe directory", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, label2->baseWidget()); n++;
    recipe_dir_choice = new FileChoice("recipe_dir", form , args, n, NULL, false);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    add_button = new Button("Add", controls, this);
    add_button->setSensitive(false);
    cancel_button = new Button("Cancel", controls, this);

    new Space("space", controls, XmHORIZONTAL, 20);
    info_label = new Label("Enter (at least) net name group", controls);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XtNwidth, 650); n++;
    XtSetArg(args[n], XtNvisibleRows, 1); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNselectable, False); n++;
    XtSetArg(args[n], XtNcenterHorizontally, True); n++;
    XtSetArg(args[n], XtNcolumns, 15); n++;
    const char *col_labels[] = {
	"net","name","type","rot","std","snr","azi","slow","phase","flo",
	"fhi","ford","zp","ftype","group"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    const char *cells[] = {
	"-","-","coh","no","0","-1.0","-1.0","-1.0","P","0.0","0.0","3","0",
	"NA","-",(char *)NULL};
    XtSetArg(args[n], XtNcells, cells); n++;
    table = new Table("table", this, args, n);
    table->addActionListener(this, XtNvalueChangedCallback);
}

AddRecipe::~AddRecipe(void)
{
}

void AddRecipe::setVisible(bool visible)
{
    info_label->setLabel("Enter (at least) net name group");

    FormDialog::setVisible(visible);

    if(visible) {
	const char *r[] = {
	"-","-","coh","no","0","-1.0","-1.0","-1.0","P","0.0","0.0","3","0",
	"NA","-"};
	table->setRow(0, r);
	table->adjustColumns();
    }
    else {
	doCallbacks(base_widget, (XtPointer)NULL, XmNactivateCallback);
    }
}

void AddRecipe::setRecipeDirChoice(void)
{
    string recipe_dir, recipe_dir2;

    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME"))) {
            recipe_dir.assign(c + string("/tables/recipes"));
        }
    }
    recipe_dir_choice->removeAllChoices();

    if(!recipe_dir.empty()) {
	recipe_dir_choice->addItem(recipe_dir);
    }
    getProperty("recipeDir2", recipe_dir2);
    if(!recipe_dir2.empty()) {
	recipe_dir_choice->addItem(recipe_dir2);
    }
}

void AddRecipe::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Cancel")) {
	setVisible(false);
	doCallbacks(base_widget, (XtPointer)NULL, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Add")) {
	if(addRecipe() ) setVisible(false);
    }
    else if(!strcmp(action_event->getReason(), XtNchangePropertyCallback)) {
	char *name = (char *)action_event->getCalldata();
	if(!strcmp(name, "recipeDir") || !strcmp(name, "recipeDir2")) {
	    setRecipeDirChoice();
        }
    }
    else if(!strcmp(action_event->getReason(), XtNvalueChangedCallback)) {
	setButtonsSensitive();
    }
}

void AddRecipe::setButtonsSensitive(void)
{
    if(table->numRows() < 1) {
	showWarning("AddRecipe.add: table has no rows.");
	return;
    }
    vector<const char *> row, cols;
    int ncols = table->getColumnLabels(cols);
    char msg[50];

    table->getRow(0, row);
    snprintf(msg, sizeof(msg), "Enter (at least)");
    int initial_length = strlen(msg);

    int i;
    for(i = 0; i < ncols; i++)
    {
	int n;
	char c[1000];
	stringcpy(c, row[i], sizeof(c));
	stringTrim(c);
	if(!strcmp(cols[i], "net")) {
	    if(strlen(c) == 0 || !strcmp(c, "-")) {
		n = strlen(msg);
		snprintf(msg+n, sizeof(msg)-n, " net");
	    }
	}
	else if(!strcmp(cols[i], "name")) {
	    if(strlen(c) == 0 || !strcmp(c, "-")) {
		n = strlen(msg);
		snprintf(msg+n, sizeof(msg)-n, " name");
	    }
	}
	else if(!strcmp(cols[i], "group")) {
	    if(strlen(c) == 0 || !strcmp(c, "-")) {
		n = strlen(msg);
		snprintf(msg+n, sizeof(msg)-n, " group");
	    }
	}
    }

    if( (int)strlen(msg) > initial_length ) {
	info_label->setLabel(msg);
	add_button->setSensitive(false);
    }
    else {
	info_label->setLabel("");
	add_button->setSensitive(true);
    }
}

bool AddRecipe::addRecipe(void)
{
    if(table->numRows() < 1) {
	showWarning("AddRecipe.add: table has no rows.");
	return false;
    }
    vector<const char *> row, cols;
    int ncols = table->getColumnLabels(cols);
    BeamRecipe recipe;

    table->getRow(0, row);

    int i;
    for(i = 0; i < ncols; i++)
    {
	int n;
	double d;
	char c[1000];
	stringcpy(c, row[i], sizeof(c));
	stringTrim(c);
	if(!strcmp(cols[i], "net")) {
	    if(strlen(c) == 0 || !strcmp(c, "-")) {
		showWarning("Must enter value for net.");
		break;
	    }
	    recipe.net.assign(stringToUpper(c));
	}
	else if(!strcmp(cols[i], "name")) {
	    if(strlen(c) == 0 || !strcmp(c, "-")) {
		showWarning("Must enter value for name.");
		break;
	    }
	    recipe.name.assign(c);
	}
	else if(!strcmp(cols[i], "type")) {
	    recipe.beam_type.assign(c);
	}
	else if(!strcmp(cols[i], "rot")) {
	    recipe.rot.assign(c);
	}
	else if(!strcmp(cols[i], "std")) {
	    if(!stringToInt(c, &n)) {
		showWarning("Must enter an integer for std.");
		break;
	    }
	    recipe.std = n;
	}
	else if(!strcmp(cols[i], "snr")) {
	    if(!stringToDouble(c, &d)) {
		showWarning("Invalid snr.");
		break;
	    }
	    recipe.snr = d;
	}
	else if(!strcmp(cols[i], "azi")) {
	    if(!stringToDouble(c, &d)) {
		showWarning("Invalid azi.");
		break;
	    }
	    recipe.azi = d;
	}
	else if(!strcmp(cols[i], "slow")) {
	    if(!stringToDouble(c, &d)) {
		showWarning("Invalid slow.");
		break;
	    }
	    recipe.slow = d;
	}
	else if(!strcmp(cols[i], "phase")) {
	    recipe.phase.assign(c);
	}
	else if(!strcmp(cols[i], "flo")) {
	    if(!stringToDouble(c, &d)) {
		showWarning("Invalid flo.");
		break;
	    }
	    recipe.flo = d;
	}
	else if(!strcmp(cols[i], "fhi")) {
	    if(!stringToDouble(c, &d)) {
		showWarning("Invalid fhi.");
		break;
	    }
	    recipe.fhi = d;
	}
	else if(!strcmp(cols[i], "ford")) {
	    if(!stringToInt(c, &n)) {
		showWarning("Must enter an integer for ford.");
		break;
	    }
	    recipe.ford = n;
	}
	else if(!strcmp(cols[i], "zp")) {
	    if(!stringToInt(c, &n)) {
		showWarning("Must enter an integer for zp.");
		break;
	    }
	    recipe.zp = n;
	}
	else if(!strcmp(cols[i], "ftype")) {
	    if(strcmp(c, "NA") && strcmp(c, "LP") && strcmp(c, "HP") &&
		strcmp(c, "BP") && strcmp(c, "BR") )
	     {
		showWarning("ftype must be NA,LP,HP,BP, or BR.");
		break;
	    }
	    recipe.ftype.assign(c);
	}
	else if(!strcmp(cols[i], "group")) {
	    if(strlen(c) == 0 || !strcmp(c, "-")) {
		showWarning("Must enter value for group.");
		break;
	    }
	    recipe.group.assign(c);
	}
    }

    if(i < ncols) return false;

    const char *recipe_dir = recipe_dir_choice->getChoice();
    bool origin_beam = (type == ORIGIN_RECIPES) ? true : false;

    if(Beam::addRecipe(recipe_dir, &recipe, origin_beam))
    {
	doCallbacks(base_widget, (XtPointer)&recipe, XmNactivateCallback);
	return true;
    }
    else {
	showWarning(GError::getMessage());
    }
    return false;
}
