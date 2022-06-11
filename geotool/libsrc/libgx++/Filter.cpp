/** \file Filter.cpp
 *  \brief Defines class Filter.
 *  \author Ivan Henson
 */
#include "config.h"
#include <sstream>

extern "C" {
#include "libstring.h"
}

#include "Filter.h"
#include "motif++/MotifClasses.h"
#include "widget/Table.h"
#include "TaperData.h"
#include "IIRFilter.h"
#include "DataMethod.h"
#include "Working.h"
#include "Waveform.h"
#include "TaperWindow.h"
#include "Demean.h"
#include "gobject++/GTimeSeries.h"

Filter::Filter(const string &name, Component *parent) : FormDialog(name, parent)
{
    data_source = NULL;
    warn = false;
    createInterface();
}

Filter::Filter(const string &name, Component *parent, DataSource *ds)
		: FormDialog(name, parent), DataReceiver(ds)
{
    warn = true;
    createInterface();
}

void Filter::createInterface(void)
{
    int n;
    Arg args[20];

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    apply_button = new Button("Apply", controls, this);
    unfilter_button = new Button("Unfilter", controls, this);
    save_button = new Button("Save", controls, this);
    save_button->setSensitive(false);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmVERTICAL); n++;
    XtSetArg(args[n], XmNpacking, XmPACK_COLUMN); n++;
    XtSetArg(args[n], XmNnumColumns, 2); n++;
    XtSetArg(args[n], XmNisAligned, True); n++;
    XtSetArg(args[n], XmNentryAlignment, XmALIGNMENT_BEGINNING); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    top_rc = new RowColumn("top_rc", this, args, n);

    input_choice = new Choice("input", top_rc);
    input_choice->addItem("Selected");
    input_choice->addItem("All");
    taper_choice = new Choice("taper", top_rc);
    taper_choice->addItem("Taper On");
    taper_choice->addItem("Taper Off");
    mode_choice = new Choice("mode", top_rc);
    mode_choice->addItem("Replace");
    mode_choice->addItem("Chain");
    taper_width_button = new Button("Taper Width...", top_rc, this);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    form = new Form("form", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    more_button = new Button("+", form, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, more_button->baseWidget()); n++;
    less_button = new Button("-", form, args, n, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget, less_button->baseWidget()); n++;
    form2= new Form("form2", form, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc1 = new RowColumn("rc1", form2, args, n);

    order_choice = new Choice("order", rc1);
    for(int i = 1; i <= 10; i++) {
	ostringstream os;
	os << i;
	order_choice->addItem(os.str());
    }
    order_choice->setChoice("3");
    type_choice = new Choice("type", rc1);
    type_choice->addItem("BP");
    type_choice->addItem("BR");
    type_choice->addItem("LP");
    type_choice->addItem("HP");
    zp_choice = new Choice("zp", rc1);
    zp_choice->addItem("ca");
    zp_choice->addItem("zp");

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc1->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc2 = new RowColumn("rc2", form2, args, n);

    low_label = new Label("low", rc2);
    XtSetArg(args[0], XmNcolumns, 8);
    low_text = new TextField("low", rc2, this, args, 1);
    new Space("space", rc2, XmHORIZONTAL, 5);
    high_label = new Label("high", rc2);
    high_text = new TextField("high", rc2, this, args, 1);

    const char *column_labels[] = {"Low", "High", "Order", "Type","Constraint"};
    const char *column_choice[] = {"","","1:2:3:4:5:6:7:8:9:10","BP:BR:LP:HP",
				"zero phase:causal"};
    const char *cells[] = {
	"0.6", "4.5", "3", "BP", "zero phase",
	"3.0", "6.0", "3", "BP", "causal",
	"2.0", "5.0", "3", "BP", "causal",
	"2.0", "4.0", "3", "BP", "causal",
	"1.0", "5.0", "3", "BP", "causal",
	"1.0", "4.0", "3", "BP", "causal",
	"1.0", "3.0", "3", "BP", "causal",
	"1.0", "2.0", "3", "BP", "causal",
	"0.5", "2.0", "3", "BP", "causal",
	"0.5", "1.5", "3", "BP", "causal",
	"6.0", "12.0", "3", "BP", "causal", (char *)NULL};

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, top_rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 15); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 10); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 10); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, form->baseWidget()); n++;
    XtSetArg(args[n], XtNtableTitle, "Filters"); n++;
    XtSetArg(args[n], XtNeditable, True); n++;
    XtSetArg(args[n], XtNradioSelect, True); n++;
    XtSetArg(args[n], XtNselectToggles, True); n++;
    XtSetArg(args[n], XtNcolumns, 5); n++;
    XtSetArg(args[n], XtNcolumnLabels, column_labels); n++;
    XtSetArg(args[n], XtNcolumnChoice, column_choice); n++;
    XtSetArg(args[n], XtNcells, cells); n++;
    XtSetArg(args[n], XtNborderWidth, 0); n++;
    table = new Table("table", this, args, n);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNvalueChangedCallback);
    table->addActionListener(this, XtNchoiceChangedCallback);

    taper_window = new TaperWindow("Filter Taper", this, 5, 5, 200);

    enableCallbackType(XmNactivateCallback);

    init();
}

Filter::~Filter(void)
{
}

void Filter::init(void)
{
    char *prop, *tok, *last;
    string selected;
    const char *r[5];
    int i;

    if( (prop = getProperty(getName() + string(".filters"))) )
    {
	table->removeAllRows();
	tok = prop;
	while((r[0]=strtok_r(tok, ",", &last)))
	{
	    tok = NULL;
	    if(!(r[1] = strtok_r(tok, ",", &last))) break;
	    if(!(r[2] = strtok_r(tok, ",", &last))) break;
	    if(!(r[3] = strtok_r(tok, ",", &last))) break;
	    if(!(r[4] = strtok_r(tok, ",", &last))) break;
	    for(i = 0; i < 5; i++) stringTrim((char *)r[i]);
	    table->addRow(r, false);
	}
	if(table->numRows() <= 0)
	{
	    r[0]="0.6"; r[1]="4.5"; r[2]="3"; r[3]="BP"; r[4]="zero phase";
	    table->addRow(r, false);
	    r[0]="3.0"; r[1]="6.0"; r[2]="3"; r[3]="BP"; r[4]="causal";
	    table->addRow(r, false);
	    r[0]="2.0"; r[1]="5.0"; r[2]="3"; r[3]="BP"; r[4]="causal";
	    table->addRow(r, false);
	    r[0]="2.0"; r[1]="4.0"; r[2]="3"; r[3]="BP"; r[4]="causal";
	    table->addRow(r, false);
	    r[0]="1.0"; r[1]="5.0"; r[2]="3"; r[3]="BP"; r[4]="causal";
	    table->addRow(r, false);
	    r[0]="1.0", r[1]="4.0", r[2]="3", r[3]="BP", r[4]="causal",
	    table->addRow(r, false);
	    r[0]="1.0", r[1]="3.0", r[2]="3", r[3]="BP", r[4]="causal",
	    table->addRow(r, false);
	    save(false);
	}
	table->adjustColumns();
	free(prop);
    }
    else {
	save(false);
    }

    if(getProperty(getName() + string(".selected"), selected))
    {
	int n;

	if(stringToInt(selected.c_str(), &n) && n > 0 && n < table->numRows())
	{
	    table->selectRow(n, true);
	}
    }
    parse_w = NULL;
}

void Filter::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(comp == low_text || comp == high_text) {
	table->deselectAllRows();
    }
    else if(!strcmp(cmd, "+")) {
	const char *r[5] = {"","","","",""};
	int n = table->numRows();
	if(n > 0) {
	    const char **row = table->getRow(n-1);
	    for(int i = 0; i < 5; i++) r[i] = row[i];
	    free(row);
	}
	table->addRow(r, true);
	setSaveSensitive();
    }
    else if(!strcmp(cmd, "-")) {
	int n = table->numRows();
	if(n > 1) {
	    table->removeRow(n-1);
	    setSaveSensitive();
	}
    }
    else if(!strcmp(cmd, "Apply")) {
	apply(false);
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	setFilter();
	apply(true);
    }
    else if(!strcmp(action_event->getReason(), XtNvalueChangedCallback) ||
	    !strcmp(action_event->getReason(), XtNchoiceChangedCallback))
    {
	edit((MmTableEditCallbackStruct *)action_event->getCalldata());
    }
    else if(!strcmp(cmd, "Taper Width...")) {
	taper_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Unfilter")) {
	unfilter();
    }
    else if(!strcmp(cmd, "Save")) {
	save(true);
    }
}

ParseCmd Filter::parseCmd(const string &cmd, string &msg)
{
    bool err;
    string c;
    double low, high;
    int order, id;
    ParseCmd ret;

    if(parseFind(cmd, "selected", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	input_choice->setChoice("Selected");
	return COMMAND_PARSED;
    }
    else if(parseFind(cmd, "all", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	input_choice->setChoice("All");
	return COMMAND_PARSED;
    }
    else if(parseFind(cmd, "taper_on", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	taper_choice->setChoice("Taper On");
	return COMMAND_PARSED;
    }
    else if(parseFind(cmd, "taper_off", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	taper_choice->setChoice("Taper Off");
	return COMMAND_PARSED;
    }
    else if(parseFind(cmd, "replace", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	mode_choice->setChoice("Replace");
	return COMMAND_PARSED;
    }
    else if(parseFind(cmd, "chain", msg, &err)) {
	if(err) return ARGUMENT_ERROR;
	mode_choice->setChoice("Chain");
	return COMMAND_PARSED;
    }
    else if(parseCompare(cmd, "select_row", 10)) {
	return table->parseCmd(cmd, msg);
    }
    else if(parseString(cmd, "table", c)) {
	return table->parseCmd(c, msg);
    }
    else if(parseString(cmd, "filter_taper", c) || parseString(cmd, "taper", c))
    {
	return taper_window->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "apply")) {
	apply(false);
	return COMMAND_PARSED;
    }

    if(parseGetArg(cmd, "wave", msg, "_wave_", &id)) {
	return parseFilter(cmd, msg);
    }
    else if(parseString(cmd, "Apply", c)) {
	return parseFilter(c, msg);
    }
    else if(parseString(cmd, "low", c) || parseString(cmd, "high", c)) {
	return parseFilter(cmd, msg);
    }
    else if(parseCompare(cmd, "Unfilter")) {
	unfilter();
    }
    else if(parseString(cmd, "Unfilter", c)) {
	if(parseGetArg(c, "wave", msg, "_wave_", &id)) {
	    if(!(parse_w = data_source->getWaveform(id))) {
		msg.assign("Waveform not found.");
		return ARGUMENT_ERROR;
	    }
	    unfilter();
	    parse_w = NULL;
	}
	else return ARGUMENT_ERROR;
    }
    else if(sscanf(cmd.c_str(), "%lf %lf %d", &low, &high, &order) == 3) {
	return autoFilter(low, high, order, "BP", false);
    }
    else if(sscanf(cmd.c_str(), "%lf %lf", &low, &high) == 2) {
	return autoFilter(low, high, 3, "BP", false);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else if((ret = table->parseCmd(cmd, msg)) != COMMAND_NOT_FOUND)
    {
	return ret;
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseCmd Filter::parseFilter(const string &cmd, string &msg)
{
    double low=0., high=0.;
    int order, id, num;
    bool zp;
    string type;
    char upper_type[3];
    ParseCmd ret;
    const char *args[] = {"type", "low", "high", "order", "zp", "_wave_"};

    num = (int)sizeof(args)/(int)sizeof(const char *);

    if(unknownArgs(cmd, msg, num, args)) return ARGUMENT_ERROR;

    if( !parseGetArg(cmd, "type", type) ) {
	type.assign("BP");
    }
    else if(!parseCompare(type, "BP") && !parseCompare(type, "BR") &&
	!parseCompare(type, "LP") && !parseCompare(type, "HP"))
    {
	msg.assign(string("filter: invalid type: ") + type);
	return ARGUMENT_ERROR;
    }
    strcpy(upper_type, type.c_str());
    stringToUpper(upper_type);

    if(strcasecmp(upper_type, "LP") &&
	!parseGetArg(cmd, "filter", msg, "low", &low))
    {
	if(msg.empty()) msg.assign("filter: missing low.");
	return ARGUMENT_ERROR;
    }
    if(strcasecmp(upper_type, "HP") &&
	!parseGetArg(cmd, "filter", msg, "high", &high))
    {
	if(msg.empty()) msg.assign("filter: missing high.");
	return ARGUMENT_ERROR;
    }
    if(!parseGetArg(cmd, "filter", msg, "order", &order)) {
	order = 3;
    }
    if(order < 0 || order > 10) {
	msg.assign("filter: invalid order (0-10).");
	return ARGUMENT_ERROR;
    }
    if(!parseGetArg(cmd, "filter", msg, "zp", &zp)) {
	zp = false;
    }
    if(parseGetArg(cmd, "filter", msg, "_wave_", &id)) {
	if(!(parse_w = data_source->getWaveform(id))) {
	    msg.assign("filter: cannot find wave object");
	    return ARGUMENT_ERROR;
	}
    }

    ret = autoFilter(low, high, order, (const char *)upper_type, zp);
    parse_w = NULL;
    return ret;
}

ParseCmd Filter::autoFilter(double low, double high, int order,
		const char *upper_type, bool zp)
{
    char s_order[20];

    low_text->setString("%.2f", low);
    high_text->setString("%.2f", high);
    snprintf(s_order, sizeof(s_order), "%d", order);
    order_choice->setChoice(s_order);
    type_choice->setChoice(upper_type);
    if(zp) {
	zp_choice->setChoice("zp");
    }
    else {
	zp_choice->setChoice("ca");
    }

    apply(true);

    return COMMAND_PARSED;
}

void Filter::parseHelp(const char *prefix)
{
    printf("%sselected\n", prefix);
    printf("%sall\n", prefix);
    printf("%staper_on\n", prefix);
    printf("%staper_off\n", prefix);
    printf("%sreplace\n", prefix);
    printf("%schain\n", prefix);
    printf("%sselect_row\n", prefix);
    printf("%sapply\n", prefix);
    printf("%sapply [type=(bp,br,lp,hp),bp] low=NUM high=NUM [order=NUM,3] [zp=(true,false),false]\n",
		prefix);
    printf("%sunfilter\n", prefix);
    printf("%stable.help\n", prefix);
    printf("%staper.help\n", prefix);
    FormDialog::parseHelp(prefix);
}

ParseVar Filter::parseVar(const string &name, string &value)
{
    string c;
    ParseVar ret;

    if(parseCompare(name, "input")) {
	value.assign(input_choice->getChoice());
    }
    else if(parseCompare(name, "taper")) {
	value.assign(taper_choice->getChoice());
    }
    else if(parseCompare(name, "mode")) {
	value.assign(mode_choice->getChoice());
    }
    else if(parseString(name, "taper", c)) {
	return taper_window->parseVar(c, value);
    }
    else if((ret = table->parseVar(name, value)) != VARIABLE_NOT_FOUND)
    {
	return ret;
    }
    else {
	return FormDialog::parseVar(name, value);
    }
    return STRING_RETURNED;
}

void Filter::save(bool permanent)
{
    string prop;
    vector<int> rows;

    makePropertyString(prop);
    putProperty(getName() + string(".filters"), prop, permanent);

    if(table->getSelectedRows(rows) > 0) {
	char s[20];
	snprintf(s, sizeof(s), "%d", rows[0]);
	putProperty(getName() + string(".selected"), s, permanent);
    }
    save_button->setSensitive(false);
}

void Filter::makePropertyString(string &prop)
{
    int j, num, len;
    vector<const char *> row;

    prop.clear();
    num = table->numRows();

    len = 0;
    for(int i = 0; i < num; i++)
    {
	table->getRow(i, row);
	for(j = 0; j < 5; j++) len += strlen(row[j]) + 1;
    }

    for(int i = 0; i < num; i++)
    {
	table->getRow(i, row);
	for(j = 0; j < 5; j++) {
	    stringTrim((char *)row[j]);
	    if(row[j][0] == '\0') break;
	}
	if(j == 5) {
	    for(j = 0; j < 5; j++) {
		prop.append(row[j]);
		prop.append(",");
	    }
	}
    }
}

void Filter::setFilter(void)
{
    vector<const char *> row;
    vector<int> rows;
    double lo_cut=0., hi_cut=0.;

    if(table->getSelectedRows(rows) <= 0) {
	return;
    }
    table->getRow(rows[0], row);

    if(strcmp(row[3], "LP")) { // not LP
	if(!stringToDouble(row[0], &lo_cut) || lo_cut < 0.) {
	    showWarning("Cannot interpret Filter Low Frequency.");
	    return;
	}
    }
    if(strcmp(row[3], "HP")) { // not HP
	if(!stringToDouble(row[1], &hi_cut) || hi_cut < 0. || hi_cut <= lo_cut)
	{
	    showWarning("Cannot interpret Filter High Frequency.");
	    return;
	}
    }
    type_choice->setChoice(row[3]);
    low_text->setString(row[0]);
    high_text->setString(row[1]);
    order_choice->setChoice(row[2]);

    if(!strncasecmp(row[4], "zero", 4)) {
	zp_choice->setChoice("zp");
    }
    else {
	zp_choice->setChoice("ca");
    }
}

void Filter::apply(bool from_table)
{
    const char *type;
    string low, high;
    bool replace, use_taper;
    int order, zero_phase, npts;
    double lo_cut = 0., hi_cut = 0., nyquist;
    gvector<Waveform *> wvec, ws;
    Working *working = NULL;

    stringToInt(order_choice->getChoice(), &order);
    type = type_choice->getChoice();
    zero_phase = !strcmp(zp_choice->getChoice(), "zp") ? 1 : 0;

    if(strcmp(type, "LP")) { // not LP
	if(!low_text->getDouble(&lo_cut) || lo_cut < 0.) {
	    showWarning("Cannot interpret Filter Low Frequency.");
	    return;
	}
    }
    if(strcmp(type, "HP")) { // not HP
	if(!high_text->getDouble(&hi_cut) || hi_cut < 0. || hi_cut <= lo_cut)
	{
	    showWarning("Cannot interpret Filter High Frequency.");
	    return;
	}
    }

    if(!strcmp(taper_choice->getChoice(), "Taper On")) use_taper = true;
    else if(!strcmp(taper_choice->getChoice(), "Taper Off")) use_taper = false;
    else {
	showWarning("Unknown taper option: %s", taper_choice->getChoice());
	return;
    }

    if(!strcmp(mode_choice->getChoice(), "Replace")) replace = true;
    else if(!strcmp(mode_choice->getChoice(), "Chain")) replace = false;
    else {
	showWarning("Cannot interpret mode: %s", mode_choice->getChoice());
	return;
    }

    if(!strcmp(input_choice->getChoice(), "All")) {
	filter.all_waveforms = true;
    }
    else {
	filter.all_waveforms = false;
    }
    filter.order = order;
    strcpy(filter.type, type);
    filter.flow = lo_cut;
    filter.fhigh = hi_cut;
    filter.zero_phase = zero_phase;
    filter.unfilter = false;
    filter.replace = replace;
    filter.use_taper = use_taper;

    doCallbacks(baseWidget(), (XtPointer)&filter, XmNactivateCallback);
    if( !data_source ) {
	if(warn) cerr << "Filter.apply: NULL data_source." << endl;
	return;
    }

    if(parse_w) {
	wvec.push_back(parse_w);
    }
    else if(!strcmp(input_choice->getChoice(), "All")) {
	data_source->getWaveforms(wvec, true);
    }
    else if(data_source->getSelectedWaveforms(wvec) <= 0)
    {
	showWarning("No waveforms selected.");
	if(from_table) {
	    table->deselectAllRows();
	}
	return;
    }

    setCursor("hourglass");

    npts = 0;
    for(int i = 0; i < wvec.size(); i++)
    {
	int j;
	for(j = 0; j < wvec[i]->size(); j++)
	{
	    nyquist = .5/wvec[i]->segment(j)->tdel();

	    if(lo_cut > nyquist && (!strcmp(type, "BP") ||
			!strcmp(type, "HP") || !strcmp(type, "BR")) )
	    {
		showWarning("Invalid Low Cut for %s/%s: nyquist=%6.3lf",
			wvec[i]->sta(), wvec[i]->chan(), nyquist);
		break;
	    }

	    if(hi_cut > nyquist && (!strcmp(type, "BP") ||
			!strcmp(type, "LP") || !strcmp(type, "BR")    ) )
	    {
		showWarning("Invalid High Cut for %s/%s: nyquist=%6.3lf",
			wvec[i]->sta(), wvec[i]->chan(), nyquist);
		break;
	    }
	}
	if(j == wvec[i]->size()) {
	    ws.push_back(wvec[i]);
	    npts += wvec[i]->length();
	}
    }

    if(ws.size() == 0) {
	setCursor("default");
	return;
    }

    int working_threshold = getProperty("filter_dialog_threshold", 1000000);

    if(npts > working_threshold) {
	char title[100];
	snprintf(title, sizeof(title), "Processing %d waveforms", ws.size());
	working = new Working("Working", this, title, "waveforms processed");
	working->setVisible(true);
    }

    try {
	for(int i = 0; i < ws.size(); i++)
	{
	    GSegment *s = ws[i]->segment(0);
	    DataMethod *dm[3];

	    if(!strcmp(type, "NA")) continue;
	    if(!strcmp(type, "LP")) lo_cut = 0.;
	    nyquist = .5/s->tdel();
	    if(!strcmp(type, "HP")) hi_cut = nyquist;
	    if(lo_cut > nyquist) lo_cut = nyquist;
	    if(hi_cut > nyquist) hi_cut = nyquist;
	    if((!strcmp(type, "BP") || !strcmp(type, "BR")) && lo_cut >= hi_cut)
	    {
		continue;
	    }

	    IIRFilter *iir = new IIRFilter(order, type, lo_cut, hi_cut,
					s->tdel(), zero_phase);
	    dm[0] = new Demean();
	    dm[1] = (DataMethod *)getTaper(use_taper);
	    dm[2] = iir;
	    wvec.push_back(ws[i]);
	    if(replace) {
		ws[i]->changeMethods(3, dm);
	    }
	    else {
		ws[i]->applyMethods(3, dm);
	    }
	    if(working && !working->update(i+1)) break;
	}
    }
    catch(...) {
	showWarning(GError::getMessage());
    }
    if(working) {
	working->destroy();
    }

    data_source->modifyWaveforms(ws);

    setCursor("default");
}

TaperData * Filter::getTaper(bool use_taper)
{
    if(use_taper)
    {
	int width, minpts, maxpts;

	if(!taper_window->width->getInt(&width) || width < 0 || width > 100) {
	    width = 5;
	    taper_window->width->setString("5");
	}
	if(!taper_window->min_points->getInt(&minpts) || minpts < 0) {
	    minpts = 5;
	    taper_window->min_points->setString("5");
	}
	if(!taper_window->max_points->getInt(&maxpts) || maxpts <= minpts) {
	    maxpts = 0;
	    taper_window->max_points->setString("");
	}
	return new TaperData("cosine", width, minpts, maxpts);
    }
    else
    {
	return new TaperData("none", 0, 0, 0);
    }
}

void Filter::unfilter(void)
{
    const char *methods[3];
    gvector<Waveform *> wvec, ws;

    if(!strcmp(input_choice->getChoice(), "All")) {
	filter.all_waveforms = true;
    }
    else {
	filter.all_waveforms = false;
    }
    filter.order = -1;
    strcpy(filter.type, "NA");
    filter.flow = 0.;
    filter.fhigh = 0.;
    filter.zero_phase = 0;
    filter.unfilter = true;
    filter.replace = false;
    filter.use_taper = false;
    doCallbacks(baseWidget(), (XtPointer)&filter, XmNactivateCallback);

    if( !data_source ) {
	if(warn) cerr << "Filter.unfilter: NULL data_source." << endl;
	return;
    }

    if(parse_w) {
	wvec.push_back(parse_w);
    }
    else if(!strcmp(input_choice->getChoice(), "All")) {
	data_source->getWaveforms(wvec, true);
    }
    else if(data_source->getSelectedWaveforms(wvec) <= 0)
    {
	showWarning("No waveforms selected.");
	return;
    }

    setCursor("hourglass");

    methods[0] = "Demean";
    methods[1] = "TaperData";
    methods[2] = "IIRFilter";

    for(int i = 0; i < wvec.size(); i++) {
	if(DataMethod::remove(3, methods, wvec[i])) {
	    ws.push_back(wvec[i]);
	}
    }

    data_source->modifyWaveforms(ws);

    table->deselectAllRows();

    setCursor("default");
}

void Filter::edit(MmTableEditCallbackStruct *c)
{
    vector<int> rows;
    int order;
    char value[32];
    double d;

    if(table->getSelectedRows(rows) > 0 && rows[0] == c->row) {
	table->deselectAllRows();
    }

    strncpy(value, c->new_string, sizeof(value));
    stringTrim(value);
    if(value[0] == '\0') return;

    if(c->column <= 1)
    {
	if(!stringToDouble(value, &d)) {
	    save_button->setSensitive(false);
	    return;
	}
    }
    else if(c->column == 2)
    {
	if(!stringToInt(value, &order) || order < 1 || order > 10) {
	    table->setField(c->row, c->column, c->old_string, true);
	}
    }
    else if(c->column == 3)
    {
	int n = strlen(value);

	if(strncasecmp(value, "BP", n) && strncasecmp(value, "BR", n) &&
	   strncasecmp(value, "HP", n) && strncasecmp(value, "LP", n))
	{
	    table->setField(c->row, c->column, c->old_string, true);
	}
    }
    else if(c->column == 4)
    {
	int n = strlen(value);

	if(strncasecmp(value, "zero phase", n) &&
	   strncasecmp(value, "causal", n))
	{
	    table->setField(c->row, c->column, c->old_string, true);
	}
    }

    setSaveSensitive();
}

void Filter::setSaveSensitive(void)
{
    string current_prop, new_prop;

    getProperty(getName() + string(".filters"), current_prop);
    makePropertyString(new_prop);

    save_button->setSensitive( current_prop.compare(new_prop) );
}
