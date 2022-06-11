/** \file TableAttributes.cpp
 *  \brief Defines class TableAttributes.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
using namespace std;

#include "widget/TableAttributes.h"
#include "widget/Table.h"
#include "motif++/MotifClasses.h"
#include "gobject++/CssTables.h"

extern "C" {
#include "libstring.h"
}

TableAttributes::TableAttributes(const string &name, Component *parent,
		const string &attribute_list) :
		FormDialog(name, parent, false, false)
{
    init(attribute_list);
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

TableAttributes::TableAttributes(const string &name, Component *parent,
		const string &attribute_list, const string &display_list) :
		FormDialog(name, parent, false, false)
{
    init(attribute_list);
    setDisplayAttributes(display_list);
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

TableAttributes::TableAttributes(const string &name, Component *parent,
		int num_members, CssClassDescription *des) :
		FormDialog(name, parent, false, false)
{
    init(num_members, des);
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

TableAttributes::TableAttributes(const string &name, Component *parent,
	int num_members, CssClassDescription *des, const string &display_list)
	: FormDialog(name, parent, false, false)
{
    init(num_members, des);
    setDisplayAttributes(display_list);
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

TableAttributes::TableAttributes(const string &name, Component *parent,
		int num_members, CssClassDescription *des, int num_extra,
		const char **extra, const char **extra_format,
		const string &display_list) :
		FormDialog(name, parent, false, false)
{
    init(num_members, des);
    getExtra(num_extra, extra, extra_format);

    setDisplayAttributes(display_list);
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

TableAttributes::TableAttributes(const string &name, Component *parent,
		int num_tables, int *num_members, CssClassDescription **des,
		int num_aux, int num_aux_members, CssClassDescription *aux_des,
		int num_extra, const char **extra, const char **extra_format,
		const string &display_list) :
		FormDialog(name, parent, false, false)
{
    init(num_tables, num_members, des, num_aux, num_aux_members, aux_des);
    getExtra(num_extra, extra, extra_format);

    setDisplayAttributes(display_list);
    createInterface();
    enableCallbackType(XmNactivateCallback);
}

void TableAttributes::getExtra(int num_extra, const char **extra,
				const char **extra_format)
{
    if(num_extra > 0)
    {
	Pixel bg[2];
	int n = num_attributes;
	num_attributes += num_extra;
	num_display_attributes = num_attributes;
	bg[0]= stringToPixel("grey80");
	bg[1] = stringToPixel("white");
	if( !colors ) {
	    colors = (Pixel *)malloc(num_attributes*sizeof(Pixel));
	    for(int i = 0; i < n; i++) {
		colors[i] = bg[0];
	    }
	}
	else {
	    colors = (Pixel *)realloc(colors, num_attributes*sizeof(Pixel));
	}
	if(n) {
	    orig_attributes = (TAttribute **)reallocWarn(orig_attributes,
				num_attributes*sizeof(TAttribute *));
	    attributes = (TAttribute **)reallocWarn(attributes,
				num_attributes*sizeof(TAttribute *));
	    test_attributes = (TAttribute **)reallocWarn(test_attributes,
				num_attributes*sizeof(TAttribute *));
	}
	else {
	    orig_attributes = (TAttribute **)mallocWarn(
				num_attributes*sizeof(TAttribute *));
	    attributes = (TAttribute **)mallocWarn(
				num_attributes*sizeof(TAttribute *));
	    test_attributes = (TAttribute **)mallocWarn(
				num_attributes*sizeof(TAttribute *));
	}
	for(int i = 0; i < num_extra; i++) {
	    orig_attributes[n+i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    orig_attributes[n+i]->name = strdup(extra[i]);
	    orig_attributes[n+i]->table = -1;
	    orig_attributes[n+i]->order = n+i;
	    stringcpy(orig_attributes[n+i]->format, extra_format[i],
		sizeof(orig_attributes[n+i]->format));
	    attributes[n+i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    attributes[n+i]->name = orig_attributes[n+i]->name;
	    attributes[n+i]->table = -1;
	    attributes[n+i]->order = n+i;
	    stringcpy(attributes[n+i]->format, extra_format[i],
		sizeof(attributes[n+i]->format));
	    test_attributes[n+i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    if(colors) {
		colors[n+i] = bg[1];
	    }
	}
    }
}

void TableAttributes::init(const string &attribute_list)
{
    num_attributes = 0;
    num_display_attributes = 0;
    orig_attributes = NULL;
    attributes = NULL;
    test_attributes = NULL;
    colors = NULL;

    createAttributes(attribute_list);

    test_attributes =
	(TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));
    for(int i = 0; i < num_attributes; i++) {
	test_attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	test_attributes[i]->name = attributes[i]->name;
	strcpy(test_attributes[i]->format, attributes[i]->format);
    }
}

void TableAttributes::init(int num_members, CssClassDescription *des)
{
    num_attributes = num_members;
    num_display_attributes = num_members;
    orig_attributes =(TAttribute **)mallocWarn(num_members*sizeof(TAttribute*));
    attributes = (TAttribute **)mallocWarn(num_members*sizeof(TAttribute *));
    colors = NULL;

    for(int i = 0; i < num_members; i++) {
	orig_attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	orig_attributes[i]->name = strdup(des[i].name);
	orig_attributes[i]->table = 0;
	orig_attributes[i]->member = i;
	orig_attributes[i]->order = i;
/*
check des.type == CSS_TIME or CSS_DATE or CSS_LDDATE or CSS_JDATE
and set attribute format to "%t", etc.
#define CSS_STRING 0
#define CSS_DOUBLE 1
#define CSS_FLOAT  2
#define CSS_INT    3
#define CSS_LONG   4
#define CSS_TIME   5
#define CSS_DATE   6
#define CSS_LDDATE 7
#define CSS_QUARK  8
#define CSS_BOOL   9
#define CSS_JDATE  10
*/
	if(des[i].type == CSS_TIME) {
	    stringcpy(orig_attributes[i]->format, "%t",
		sizeof(orig_attributes[i]->format));
	}
	else {
	    stringcpy(orig_attributes[i]->format, des[i].format,
		sizeof(orig_attributes[i]->format));
	}
	attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	attributes[i]->name = orig_attributes[i]->name;
	attributes[i]->table = 0;
	attributes[i]->member = i;
	attributes[i]->order = i;
	stringcpy(attributes[i]->format, orig_attributes[i]->format,
		sizeof(attributes[i]->format));
    }

    test_attributes =
	(TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));
    for(int i = 0; i < num_attributes; i++) {
	test_attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	test_attributes[i]->name = attributes[i]->name;
	strcpy(test_attributes[i]->format, attributes[i]->format);
    }
}

void TableAttributes::init(int num_tables, int *num_members,
			CssClassDescription **des, int num_aux, int num_aux_members,
			CssClassDescription *aux_des)
{
    Pixel bg[3];

    num_attributes = 0;
    for(int i = 0; i < num_tables; i++) {
	num_attributes += num_members[i];
    }
    num_attributes += num_aux*num_aux_members;
    num_display_attributes = num_attributes;
    orig_attributes = (TAttribute **)mallocWarn(
				num_attributes*sizeof(TAttribute *));
    attributes = (TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));
    colors = (Pixel *)mallocWarn(num_attributes*sizeof(Pixel));
    bg[0] = stringToPixel("grey80");
    bg[1] = stringToPixel("grey60");
    bg[2] = stringToPixel("white");

    int n = 0;
    for(int i = 0; i < num_tables; i++)
    {
	CssClassDescription *ds = des[i];
	for(int j = 0; j < num_members[i]; j++)
	{
	    orig_attributes[n] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    orig_attributes[n]->name = strdup(ds[j].name);
	    orig_attributes[n]->table = i;
	    orig_attributes[n]->member = j;
	    orig_attributes[n]->order = n;
	    if(ds[j].type == CSS_TIME) {
		stringcpy(orig_attributes[n]->format, "%t",
			sizeof(orig_attributes[n]->format));
	    }
	    else {
		stringcpy(orig_attributes[n]->format, ds[j].format,
			sizeof(orig_attributes[n]->format));
	    }
	    attributes[n] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    attributes[n]->name = orig_attributes[n]->name;
	    attributes[n]->table = i;
	    attributes[n]->member = j;
	    attributes[n]->order = n;
	    stringcpy(attributes[n]->format, orig_attributes[n]->format,
			sizeof(attributes[n]->format));
	    colors[n] = (i < 3) ? bg[i] : bg[0];
	    n++;
	}
    }
    char buf[100];
    for(int j = 0; j < num_aux_members; j++)
    {
	for(int i = 0; i < num_aux && i < 99; i++)
	{
	    orig_attributes[n] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    snprintf(buf, sizeof(buf), "%s%02d", aux_des[j].name, i+1);
	    orig_attributes[n]->name = strdup(buf);
	    orig_attributes[n]->table = num_tables + i;
	    orig_attributes[n]->member = j;
	    orig_attributes[n]->order = n;
	    if(aux_des[j].type == CSS_TIME) {
		stringcpy(orig_attributes[n]->format, "%t",
			sizeof(orig_attributes[n]->format));
	    }
	    else {
		stringcpy(orig_attributes[n]->format, aux_des[j].format,
			sizeof(orig_attributes[n]->format));
	    }
	    attributes[n] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	    attributes[n]->name = orig_attributes[n]->name;
	    attributes[n]->table = num_tables + i;
	    attributes[n]->member = j;
	    attributes[n]->order = n;
	    stringcpy(attributes[n]->format, orig_attributes[n]->format,
			sizeof(attributes[n]->format));
	    colors[n] = bg[2];
	    n++;
	}
    }

    test_attributes =
	(TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));
    for(int i = 0; i < num_attributes; i++) {
	test_attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	test_attributes[i]->name = attributes[i]->name;
	strcpy(test_attributes[i]->format, attributes[i]->format);
    }
}

TableAttributes::~TableAttributes(void)
{
    for(int i = 0; i < num_attributes; i++) {
	free(orig_attributes[i]->name);
	Free(orig_attributes[i]);
	Free(attributes[i]);
	Free(test_attributes[i]);
    }
    Free(orig_attributes);
    Free(attributes);
    Free(test_attributes);
    Free(colors);
}

void TableAttributes::createInterface(void)
{
    Arg args[20];
    const char **column_labels=NULL;
    int n;

    setSize(800, 125);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
//    apply_button->setSensitive(false);
    reset_button = new Button("Reset", controls, this);
    reset_button->setSensitive(false);
    select_all_button = new Button("Select All", controls, this);
    deselect_all_button = new Button("Deselect All", controls, this);

    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    sep = new Separator("sep", this, args, n);

    if(!num_attributes) {
	cerr << "TableAttributes " << component_name
		<< "warning: number of attributes = 0" << endl;
    }
    else {
	column_labels =(const char **)mallocWarn(num_attributes*sizeof(char *));
    }

    for(int i = 0; i < num_attributes; i++) {
	column_labels[i] = attributes[i]->name;
    }

    n = 0;
    XtSetArg(args[n], XtNtableTitle, getName()); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XtNbottomMargin, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNcenterHorizontally, true); n++;
    XtSetArg(args[n], (char *)XtNdisplayVerticalScrollbar, False); n++;
    XtSetArg(args[n], (char *)XtNdisplayHorizontalScrollbar, False); n++;
    if(num_attributes) {
	XtSetArg(args[n], (char *)XtNcolumns, num_attributes); n++;
	XtSetArg(args[n], (char *)XtNcolumnLabels, column_labels); n++;
    }
    XtSetArg(args[n], (char *)XtNeditable, true); n++;
    XtSetArg(args[n], (char *)XtNrowSelectable, false); n++;
    XtSetArg(args[n], (char *)XtNcolumnSelectable, true); n++;
    XtSetArg(args[n], (char *)XtNcolumnSingleSelect, false); n++;

    table = new Table("table", this, args, n);

    list();
    Free(column_labels);

    table->addActionListener(this, XtNcolumnMovedCallback);
    table->addActionListener(this, XtNselectColumnCallback);
    table->addActionListener(this, XtNleaveWindowCallback);
}

void TableAttributes::list(void)
{
    vector<const char *> row;
    vector<bool> states;
    vector<int> cols;
    char **labels = NULL;
    Arg args[2];

    table->removeRow(0);
    if(!num_attributes) {
	return;
    }
    labels = (char **)mallocWarn(num_attributes*sizeof(char *));
    for(int i = 0; i < num_attributes; i++) {
	cols.push_back(i);
	states.push_back((i < num_display_attributes) ? true : false);
        labels[i] = attributes[i]->name;
	row.push_back(attributes[i]->format);
    }
    XtSetArg(args[0], XtNcolumns, num_attributes);
    XtSetArg(args[1], XtNcolumnLabels, labels);
    table->setValues(args, 2);

    table->addRow(row, true);
    table->selectColumns(cols, states);
    if(colors) table->setColumnColors(colors);

    Free(labels);
}

void TableAttributes::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
	list();
//	apply_button->setSensitive(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
//	apply_button->setSensitive(false);
        reset_button->setSensitive(true);
	doCallbacks(base_widget, (XtPointer)this, XmNactivateCallback);
    }
    else if(!strcmp(cmd, "Reset")) {
	for(int i = 0; i < num_attributes; i++) {
	    attributes[i]->name = orig_attributes[i]->name;
	    attributes[i]->order = i;
	    attributes[i]->table = orig_attributes[i]->table;
	    attributes[i]->member = orig_attributes[i]->member;
	    strcpy(attributes[i]->format, orig_attributes[i]->format);
	    num_display_attributes = num_attributes;
	}
	list();
        reset_button->setSensitive(false);
    }
    else if(!strcmp(cmd, "Select All")) {
	table->selectAllColumns(true);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	table->selectAllColumns(false);
    }
    else if(!strcmp(cmd, "table")) {
//	apply_button->setSensitive(changed());
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Table Attributes Help");
    }
}

ParseCmd TableAttributes::parseCmd(const string &cmd, string &msg)
{
    ParseCmd ret;

    if(parseCompare(cmd, "Close")) {
	close_button->activate();
    }
    else if(parseCompare(cmd, "Apply")) {
	apply_button->activate();
    }
    else if(parseCompare(cmd, "Reset")) {
	reset_button->activate();
    }
    else if(parseCompare(cmd, "Select_All")) {
	select_all_button->activate();
    }
    else if(parseCompare(cmd, "Deselect_All")) {
	deselect_all_button->activate();
    }
    else if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND) {
	return ret;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

void TableAttributes::parseHelp(const char *prefix)
{
    printf("%sapply\n", prefix);
    printf("%sreset\n", prefix);
    printf("%sselect all\n", prefix);
    printf("%sdeselect all\n", prefix);
    char p[200];
    snprintf(p, sizeof(p), "%sattributes.", prefix);
    Table::parseHelp(p);
}

void TableAttributes::createAttributes(const string &attribute_list)
{
    int i, n;
    char *c, *buf, *tok, *last;

    for(i = 0; i < num_attributes; i++) {
	free(orig_attributes[i]->name);
	Free(orig_attributes[i]);
	Free(attributes[i]);
    }
    Free(orig_attributes);
    Free(attributes);
    num_attributes = 0;
    num_display_attributes = 0;
    if( attribute_list.empty() ) return;

    buf = strdup(attribute_list.c_str());
    tok = buf;
    n = 0;
    while((c = strtok_r(tok, ",;:", &last)) != NULL)
    {
	tok = NULL;
	if((c = strtok_r(NULL, ",;:", &last)) == NULL
		|| strchr(c, '%') == NULL) break;
	n++;
    }
    num_attributes = n;

    orig_attributes = (TAttribute **)mallocWarn(n*sizeof(TAttribute *));
    attributes = (TAttribute **)mallocWarn(n*sizeof(TAttribute *));

    for(i = 0; i < n; i++) {
	orig_attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
	attributes[i] = (TAttribute *)mallocWarn(sizeof(TAttribute));
    }

    stringcpy(buf, attribute_list.c_str(), attribute_list.length()+1);

    tok = buf;
    for(i = 0; i < n; i++)
    {
	c = strtok_r(tok, ",;:", &last);
	orig_attributes[i]->name = strdup(stringTrim(c));
	c = strtok_r(NULL, ",;:", &last);
	stringcpy(orig_attributes[i]->format, stringTrim(c),
		sizeof(orig_attributes[i]->format));
	orig_attributes[i]->table = -1;
	orig_attributes[i]->member = i;
	orig_attributes[i]->order = i;
	tok = NULL;
    }
    free(buf);

    for(i = 0; i < n; i++) {
	attributes[i]->name = orig_attributes[i]->name;
	strcpy(attributes[i]->format, orig_attributes[i]->format);
	attributes[i]->table = -1;
	attributes[i]->member = i;
	attributes[i]->order = i;
    }
    num_display_attributes = num_attributes;
}

void TableAttributes::setDisplayAttributes(const string &display_list)
{
    int i, lastn, n;
    char *c, *buf, *tok, *last;
    TAttribute **attr = NULL;

    if(!num_attributes || display_list.empty() ) return;

    attr = (TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));

    /* reorder attributes[] according to the display string
     */
    for(i = 0; i < num_attributes; i++) {
	attr[i] = attributes[i];
    }

    buf = strdup(display_list.c_str());
    tok = buf;

    n = 0;
    lastn = -1;
    while((c = strtok_r(tok, ",;:", &last)) != NULL)
    {
	tok = NULL;
	stringTrim(c);
	if(c[0] != '%') {
	    for(i = 0; i < num_attributes; i++) {
		if(attr[i] != NULL && !strcasecmp(c, attr[i]->name)) {
		    lastn = n;
		    attributes[n++] = attr[i];
		    attr[i] = NULL;
		    break;
		}
	    }
	}
	else if(lastn >= 0) {
	    stringcpy(attributes[lastn]->format, c,
			sizeof(attributes[lastn]->format));
	    lastn = -1;
	}
    }
    free(buf);

    num_display_attributes = (n > 0) ? n : num_attributes; 
    for(i = 0; i < num_attributes; i++) {
	if(attr[i] != NULL) {
	    attributes[n++] = attr[i];
	}
    }
    Free(attr);
}

void TableAttributes::apply(void)
{
    vector<const char *> format;
    char fmt[50];
    int i, j, n;
    int n_selected;
    vector<int> cols, order;
    TAttribute **attr = NULL;

    /* get format changes */
    table->getRow(0, format);
    for(i = 0; i < num_attributes; i++) {
	stringTrimCopy(fmt, (char *)format[i], sizeof(fmt));
	stringcpy(attributes[i]->format, fmt, sizeof(attributes[i]->format));
    }

    table->promoteSelectedColumns();

    n = table->numColumns();
    if(n != num_attributes) {
	showWarning("TableAttributes::apply: error.");
	return;
    }
    n_selected = table->getSelectedColumns(cols);
    table->getColumnOrder(order);

//    attr = (TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));
    attr = new TAttribute *[num_attributes];
    for(i = 0; i < num_attributes; i++) {
	attr[i] = attributes[i];
    }

    n = 0;
    for(i = 0; i < num_attributes; i++) {
	for(j = 0; j < n_selected; j++) {
	    if(order[i] == cols[j]) break;
	}
	if(j < n_selected) {
	    attributes[n++] = attr[order[i]];
	    attr[order[i]] = NULL;
	}
    }
    for(i = 0; i < num_attributes; i++) {
	if(attr[i] != NULL) {
	    attributes[n++] = attr[i];
	}
    }
//    Free(attr);
    delete [] attr;

    num_display_attributes = (n_selected > 0) ? n_selected : num_attributes;

    table->resetColumnOrder();

/*
    AttributeList(w, gt);

    (*gt->listTable)(gt);

    saveTableAttributes(programProperties(), gt);
    writeApplicationProperties();
*/
}

bool TableAttributes::changed(void)
{
    vector<const char *> format;
    char fmt[50];
    int i, j, n;
    int n_selected;
    vector<int> cols, order;
    TAttribute **attr = NULL;

    n = n_selected = table->getSelectedColumns(cols);
    if(n == 0) n = num_attributes;
    if(n != num_display_attributes) return true;

    /* get format changes */
    table->getRow(0, format);
    for(i = 0; i < num_attributes; i++) {
	test_attributes[i]->name = attributes[i]->name;
	stringTrimCopy(fmt, (char *)format[i], sizeof(fmt));
	stringcpy(test_attributes[i]->format, fmt,
                        sizeof(test_attributes[i]->format));
    }

    n = table->numColumns();
    if(n != num_attributes) {
	showWarning("TableAttributes::apply: error.");
	return false;
    }
    table->getColumnOrder(order);

//    attr = (TAttribute **)mallocWarn(num_attributes*sizeof(TAttribute *));
    attr = new TAttribute *[num_attributes];
    for(i = 0; i < num_attributes; i++) {
	attr[i] = test_attributes[i];
    }

    n = 0;
    for(i = 0; i < num_attributes; i++) {
	for(j = 0; j < n_selected; j++) {
	    if(order[i] == cols[j]) break;
	}
	if(j < n_selected) {
	    test_attributes[n++] = attr[order[i]];
	    attr[order[i]] = NULL;
	}
    }
    for(i = 0; i < num_attributes; i++) {
	if(attr[i] != NULL) {
	    test_attributes[n++] = attr[i];
	}
    }
//    Free(attr);
    delete [] attr;

    n = (n_selected > 0) ? n_selected : num_attributes;

    for(i = 0; i < n; i++) {
	if(strcmp(test_attributes[i]->name, attributes[i]->name) ||
	    strcmp(test_attributes[i]->format, attributes[i]->format))
	{
	    return true;
	}
    }
    return false;
}

void TableAttributes::setOrder(vector<int> &order)
{
    TAttribute **attr = (TAttribute **)mallocWarn(
                        num_display_attributes*sizeof(TAttribute *));

    for(int i = 0; i < num_display_attributes; i++) {
        attr[i] = attributes[i];
    }
    for(int i = 0; i < num_display_attributes && i < (int)order.size(); i++) {
        attributes[i] = attr[order[i]];
    }
    free(attr);
    list();
}

TAttribute TableAttributes::getAttribute(int i)
{
    TAttribute a = {NULL, "", -1};

    if(i >= 0 && i < num_attributes) {
	return *attributes[i];
    }
    return a;
}

TAttribute TableAttributes::getAttribute(const string &name)
{
    TAttribute a = {NULL, "", -1};

    for(int i = 0; i < num_attributes; i++) {
	if(!name.compare(attributes[i]->name)) {
	    return *attributes[i];
	}
    }
    return a;
}
