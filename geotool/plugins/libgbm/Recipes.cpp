/** \file Recipes.cpp
 *  \brief Defines class Recipes
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <math.h>

using namespace std;

#include "Recipes.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "libgio.h"
#include "Slowness.h"
#include "IIRFilter.h"
#include "BeamGroups.h"
#include "AddRecipe.h"
#include "gobject++/CssTables.h"
#include "cssio.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
static int sort_by_netorigin(const void *A, const void *B);
}

using namespace libgbm;

Recipes::Recipes(const char *name, Component *parent, RecipeType recipe_type,
		DataSource *ds) : Frame(name, parent), DataReceiver(ds)
{
    if(data_source) {
	wp = data_source->getWaveformPlotInstance();
    }
    else wp = NULL;

    type = recipe_type;
    createInterface();
    init();
}

void Recipes::createInterface()
{
    int n;
    Arg args[20];

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    add_button = new Button("Add Recipe...", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);
    edit_button = new Button("Edit", edit_menu, this);

    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);
    deselect_all_button = new Button("Deselect All", view_menu, this);
    beam_window_toggle = new Toggle("Beam Window", view_menu, this);
    replace_beam_toggle = new Toggle("Replace Beam", view_menu, this);
    replace_beam_toggle->set(true);

    option_menu = new Menu("Option", menu_bar);
    beam_button = new Button("Beam", option_menu, this);
    beam_selected_button = new Button("Beam Selected", option_menu, this);
    groups_button = new Button("Groups...", option_menu, this);
    location_menu = new Menu("Beam Location", option_menu, true);
    dnorth_toggle = new Toggle("dnorth/deast", location_menu, this);
    dnorth_toggle->set(true);
    reference_toggle = new Toggle("reference station", location_menu, this);
    array_center_toggle = new Toggle("array center", location_menu, this);
    ref_stations_button = new Button("Reference Stations...", option_menu,this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    beam_help_button = new Button("Beam Help", help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XtNwidth, 580); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XtNcntrlSelect, True); n++;
    XtSetArg(args[n], XmNmarginHeight, 1); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    if(type == ORIGIN_RECIPES) {
	XtSetArg(args[n], XtNtableTitle, "Origin Beam Recipes"); n++;
    }
    else {
	XtSetArg(args[n], XtNtableTitle, "Detection Beam Recipes"); n++;
    }
    XtSetArg(args[n], XtNcolumns, 16); n++;
    table = new Table("Origin Beam Recipes", frame_form, info_area, args, n);
    table->setAttributes(
"net,%s,name,%s,type,%s,rot,%s,std,%.1f,snr,%.2f,azi,%.2f,slow,%.2f,phase,%s,\
flo,%.2f,fhi,%.2f,ford,%d,zp,%d,ftype,%s,group,%s,path,%s");
 
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);
    table->addActionListener(this, XtNeditSaveCallback);

    beam_window = NULL;
    ref_sta_window = NULL;
    add_recipe_window = NULL;

    addPlugins("Recipes", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	tool_bar->add(beam_button, "Beam");
	tool_bar->add(beam_selected_button, "Beam Selected");
	tool_bar->add(groups_button, "Groups");
	tool_bar->add(beam_window_toggle, "BW");
	tool_bar->add(replace_beam_toggle, "Replace");
    }
}

Recipes::~Recipes(void)
{
    delete gbeam;
}

void Recipes::init(void)
{
    string recipe_dir, recipe_dir2;

    recipes = NULL;
    beam_groups = NULL;
    travel_time = NULL;

    if( wp ) wp->addActionListener(this, XtNdataChangeCallback);

    if(!getProperty("recipeDir", recipe_dir)) {
	const char *c;
	if((c = Application::getInstallDir("GEOTOOL_HOME")) != NULL) {
	    recipe_dir.assign(c + string("/tables/recipes"));
	}
    }
    getProperty("recipeDir", recipe_dir2);

    gbeam = new Beam(recipe_dir, recipe_dir2);

    readRecipes(false);

    list();
}

void Recipes::setDataSource(DataSource *ds)
{
    if(ds != data_source)
    {
	if(data_source) {
	    data_source->removeDataReceiver(this);
	    if(wp) {
		wp->removeActionListener(this, XtNdataChangeCallback);
	    }
	}
	data_source = ds;
	if(data_source) {
	    data_source->addDataReceiver(this);
	    wp = data_source->getWaveformPlotInstance();
	    if(wp) wp->addActionListener(this, XtNdataChangeCallback);
	}
        else wp = NULL;
    }

    setButtonsSensitive();

    if(ref_sta_window && ref_sta_window->isVisible()) {
	loadRefSta();
    }
}

void Recipes::readRecipes(bool force_read)
{
    if(type == ORIGIN_RECIPES) {
	recipes = gbeam->getOriginRecipes(force_read);
    }
    else {
	recipes = gbeam->getDetectionRecipes(force_read);
    }
}

void Recipes::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Add Recipe...")) {
	if(!add_recipe_window) {
	    add_recipe_window =
		new AddRecipe("Add Beam Recipe", this, type, this);
	}
	add_recipe_window->setVisible(true);
	menu_bar->setSensitive(false);
	tool_bar->setSensitive(false);
    }
    else if(comp == add_recipe_window) {
	if(action_event->getCalldata() != NULL) {
	    readRecipes(true);
	    list();
	}
	menu_bar->setSensitive(true);
	tool_bar->setSensitive(true);
    }
    else if(!strcmp(cmd, "Delete")) {
	deleteRecipe();
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Edit")) {
        table->editModeOn();
    }
    else if(!strcmp(action_event->getReason(), XtNeditSaveCallback)) {
	saveRecipe();
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Deselect All")) {
	table->deselectAllRows();
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Beam")) {
	beamWaveforms(false);
    }
    else if(!strcmp(cmd, "Beam Selected")) {
	beamWaveforms(true);
    }
    else if(!strcmp(cmd, "Groups...")) {
	if(!beam_groups) {
	    beam_groups = new BeamGroups("Beam Groups", this);
	}
	beam_groups->setVisible(true);
    }
    else if(!strcmp(cmd, "Reference Stations...")) {
	loadRefSta();
    }
    else if(!strcmp(cmd, "Beam Help")) {
	showHelp(cmd);
    }
    else if(!strcmp(action_event->getReason(), XtNattributeChangeCallback)) {
	list();
    }
    else if(!strcmp(action_event->getReason(), XtNselectRowCallback)) {
	selectRecipe((MmTableSelectCallbackStruct *)action_event->getCalldata());
	if(ref_sta_window && ref_sta_window->isVisible()) {
	    loadRefSta();
	}
    }
    else if(!strcmp(action_event->getReason(), XtNdataChangeCallback)) {
	setButtonsSensitive();
	if(ref_sta_window && ref_sta_window->isVisible()) {
	    DataChange *c = (DataChange *)action_event->getCalldata();
	    if(c->waveform) { // need specifically waveform added or deleted!!
		loadRefSta();
	    }
	}
    }
}

ParseCmd Recipes::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseString(cmd, "table", c)) {
	return table->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Beam")) {
	beam_button->activate();
    }
    else if(parseCompare(cmd, "Beam_Selected")) {
	beam_selected_button->activate();
    }
    else if(parseArg(cmd, "Beam_Location", c)) {
	return location_menu->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Beam_Window", c)) {
	return beam_window_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "Replace_Beam", c)) {
	return replace_beam_toggle->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "beams", c)) {
	if(!beam_window) {
	    beam_window = new WaveformWindow("Beams", this);
	}
	return beam_window->parseCmd(c, msg);
    }
    else if(parseArg(cmd, "beam_groups", c)) {
	if(!beam_groups) {
	    beam_groups = new BeamGroups("Beam Groups", this);
	}
	return beam_groups->parseCmd(c, msg);
    }
    else if(parseCompare(cmd, "Help")) {
	char prefix[200];
	getParsePrefix(prefix, sizeof(prefix));
	parseHelp(prefix);
    }
    else {
	return FormDialog::parseCmd(cmd, msg);
    }
    return COMMAND_PARSED;
}

ParseVar Recipes::parseVar(const string &name, string &value)
{
    string s;

    if(parseString(name, "table", s)) {
	return table->parseVar(s, value);
    }
    else if(parseString(name, "beams", s)) {
	if(beam_window) {
	    return beam_window->parseVar(s, value);
	}
	else {
	    value.assign("Beam Window not found.");
	    return VARIABLE_ERROR;
	}
    }
    return Frame::parseVar(name, value);
}

void Recipes::parseHelp(const char *prefix)
{
    Table::parseHelp(prefix);
    printf("%sbeam\n", prefix);
    printf("%sbeam_selected\n", prefix);
    printf("%sbeam_location=(dnorth/deast,reference station,array center)\n",
		prefix);
    printf("%sbeam_window=(true,false)\n", prefix);
    printf("%sreplace_beam=(true,false)\n", prefix);
}

void Recipes::setButtonsSensitive(void)
{
    bool recipe_selected = table->numSelectedRows() > 0 ? true : false;
    bool waveform_selected = (wp && wp->numSelected()) ? true : false;

    beam_button->setSensitive(recipe_selected);
    deselect_all_button->setSensitive(recipe_selected);
    delete_button->setSensitive(recipe_selected);
    edit_button->setSensitive(recipe_selected);

    beam_selected_button->setSensitive(recipe_selected && waveform_selected);
    ref_stations_button->setSensitive(recipe_selected && waveform_selected);
}

void Recipes::list(void)
{
    int num_columns;
    const char *row[16];
    char std[20], snr[20], azi[20], slow[20], flo[20], fhi[20];
    char ford[20], zp[20];
    bool editable[16];
    TAttribute a[20];

    if(!recipes) return;

    setCursor("hourglass");

    table->removeAllRows();
    num_columns = table->numColumns();

    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
    }

    for(int i = 0; i < (int)recipes->size(); i++)
    {
	for(int j = 0; j < num_columns; j++)
	{
	    editable[j] = true;
	    row[j] = "-";

	    if(!strcmp(a[j].name, "net")) {
		editable[j] = false;
		row[j] = (char *)recipes->at(i).net.c_str();
	    }
	    else if(!strcmp(a[j].name, "name")) {
		editable[j] = false;
		row[j] = (char *)recipes->at(i).name.c_str();
	    }
	    else if(!strcmp(a[j].name, "type")) {
		row[j] = (char *)recipes->at(i).beam_type.c_str();
	    }
	    else if(!strcmp(a[j].name, "rot")) {
		row[j] = (char *)recipes->at(i).rot.c_str();
	    }
	    else if(!strcmp(a[j].name, "std")) {
		snprintf(std, 20, "%d", recipes->at(i).std);
		row[j] = std;
	    }
	    else if(!strcmp(a[j].name, "snr")) {
		snprintf(snr, 20, a[j].format, recipes->at(i).snr);
		row[j] = snr;
	    }
	    else if(!strcmp(a[j].name, "azi")) {
		snprintf(azi, 20, a[j].format, recipes->at(i).azi);
		row[j] = azi;
	    }
	    else if(!strcmp(a[j].name, "slow")) {
		snprintf(slow, 20, a[j].format, recipes->at(i).slow);
		row[j] = slow;
	    }
	    else if(!strcmp(a[j].name, "phase")) {
		row[j] = (char *)recipes->at(i).phase.c_str();
	    }
	    else if(!strcmp(a[j].name, "flo")) {
		snprintf(flo, 20, a[j].format, recipes->at(i).flo);
		row[j] = flo;
	    }
	    else if(!strcmp(a[j].name, "fhi")) {
		snprintf(fhi, 20, a[j].format, recipes->at(i).fhi);
		row[j] = fhi;
	    }
	    else if(!strcmp(a[j].name, "ford")) {
		snprintf(ford, 20, "%d", recipes->at(i).ford);
		row[j] = ford;
	    }
	    else if(!strcmp(a[j].name, "zp")) {
		snprintf(zp, 20, "%d", recipes->at(i).zp);
		row[j] = zp;
	    }
	    else if(!strcmp(a[j].name, "ftype")) {
		row[j] = (char *)recipes->at(i).ftype.c_str();
	    }
	    else if(!strcmp(a[j].name, "group")) {
		row[j] = (char *)recipes->at(i).group.c_str();
	    }
	    else if(!strcmp(a[j].name, "path")) {
		editable[j] = false;
		row[j] = quarkToString(recipes->at(i).path);
	    }
	}
	table->addRow(row, false);
    }

    table->adjustColumns();
    table->sortByColumnLabels("net,name");

    table->setColumnEditable(editable);

    for(int i = 0; i < (int)recipes->size(); i++)
    {
	if(recipes->at(i).selected) {
	    table->selectRow(i, true);
	}
    }

    setButtonsSensitive();

    setCursor("default");
}

void Recipes::saveRecipe(void)
{
    TAttribute a[20];
    vector<const char *> row;
    int num_rows = table->numRows();
    int num_columns = table->numColumns();

    for(int i = 0; i < num_columns; i++) {
	a[i] = table->getAttribute(i);
    }

    for(int i = 0; i < num_rows; i++) if(table->rowEdited(i))
    {
	BeamRecipe *recipe = &recipes->at(i);
	table->getRow(i, row);

	for(int j = 0; j < num_columns; j++) if(table->fieldEdited(i, j))
	{
	    int n;
	    double d;
	    if(!strcmp(a[j].name, "type")) {
		recipe->beam_type.assign(row[j]);
	    }
	    else if(!strcmp(a[j].name, "rot")) {
		recipe->rot.assign(row[j]);
	    }
	    else if(!strcmp(a[j].name, "std")) {
		if(!stringToInt(row[j], &n)) {
		    showWarning("Invalid value for std.");
		}
		recipe->std = n;
	    }
	    else if(!strcmp(a[j].name, "snr")) {
		if(!stringToDouble(row[j], &d)) {
		    showWarning("Invalid value for snr.");
		}
		recipe->snr = d;
	    }
	    else if(!strcmp(a[j].name, "azi")) {
		if(!stringToDouble(row[j], &d)) {
		    showWarning("Invalid value for azi.");
		}
		recipe->azi = d;
	    }
	    else if(!strcmp(a[j].name, "slow")) {
		if(!stringToDouble(row[j], &d)) {
		    showWarning("Invalid value for slow.");
		}
		recipe->slow = d;
	    }
	    else if(!strcmp(a[j].name, "phase")) {
		recipe->phase.assign(row[j]);
	    }
	    else if(!strcmp(a[j].name, "flo")) {
		if(!stringToDouble(row[j], &d)) {
		    showWarning("Invalid value for flo.");
		}
		recipe->flo = d;
	    }
	    else if(!strcmp(a[j].name, "fhi")) {
		if(!stringToDouble(row[j], &d)) {
		    showWarning("Invalid value for fhi.");
		}
		recipe->fhi = d;
	    }
	    else if(!strcmp(a[j].name, "ford")) {
		if(!stringToInt(row[j], &n)) {
		    showWarning("Invalid value for ford.");
		}
		recipe->ford = n;
	    }
	    else if(!strcmp(a[j].name, "zp")) {
		if(!stringToInt(row[j], &n)) {
		    showWarning("Invalid value for zp.");
		}
		recipe->zp = n;
	    }
	    else if(!strcmp(a[j].name, "ftype")) {
		recipe->ftype.assign(row[j]);
	    }
	    else if(!strcmp(a[j].name, "group")) {
		recipe->group.assign(row[j]);
	    }
	}
	if( !Beam::changeRecipe(recipe, False) ) {
	    showWarning(GError::getMessage());
	}
    }
    table->backup();
}

void Recipes::selectRecipe(MmTableSelectCallbackStruct *c)
{
    int i, j, n;
    ostringstream os;

    if(recipes == NULL || (int)recipes->size() == 0) { // should not happen
	return;
    }

    for(i = 0; i < c->nchanged_rows; i++)
    {
	/* can only select one recipe per array (net)
	 */
	if(c->states[c->changed_rows[i]]) {
	    for(j = 0; j < c->nrows; j++) {
		if(j != c->changed_rows[i] && c->states[j] &&
		    !recipes->at(c->changed_rows[i]).net.compare(
				recipes->at(j).net))
		{
		    table->selectRow(j, false);
		    c->states[j] = false;
		}
	    }
	}
    }
    for(i = n = 0; i < c->nrows; i++) {
	if(c->states[i]) {
	    n += (int)recipes->at(i).net.length() +
			(int)recipes->at(i).name.length() + 2;
	}
    }
    for(i = n = 0; i < c->nrows; i++) {
	if(c->states[i]) {
	    os << recipes->at(i).net << "/" << recipes->at(i).name << ",";
	    recipes->at(i).selected = true;
	}
	else {
	    recipes->at(i).selected = false;
	}
    }

    setButtonsSensitive();

    if(type == ORIGIN_RECIPES) {
	putProperty("originRecipeSelected", os.str());
    }
    else {
	putProperty("detectionRecipeSelected", os.str());
    }
    Application::writeApplicationProperties();
}

void Recipes::beamWaveforms(bool selected_only)
{
    int num_selected;
    vector<int> rows;
    bool beamed_one;
    gvector<Waveform *> wvec;

    if(!wp) return;

    if( (num_selected = table->getSelectedRows(rows)) <= 0)
    {
	showWarning("No beam recipes selected.");
	return;
    }
    if(selected_only) {
	if(wp->getSelectedWaveforms(wvec) <= 0){
	    showWarning("No waveforms selected.");
	    return;
	}
    }
    else if(wp->getWaveforms(wvec) <= 0)
    {
	showWarning("No waveforms.");
	return;
    }

    setCursor("hourglass");

    /* sort waveforms by network, then origin.
     */
    wvec.sort(sort_by_netorigin);

    /* loop over selected recipes
     */
    beamed_one = false;
    for(int i = 0; i < num_selected; i++)
    {
	bool found_waveform = false, need_origin = true;
	BeamRecipe *recipe = &recipes->at(rows[i]);
	CssOriginClass *origin, *last_origin = (CssOriginClass *)-1;

	/* loop over all or selected waveforms. Find waveforms from this
	 * recipe->net and beam each unique origin.
	 */
	/* check if an origin is required
	 */
	if( recipe->azi != -1. && recipe->slow >= 0. )
	{
	    need_origin = false;
	}

	if(!need_origin)
	{
	    /* if we don't need an origin for azimuth and slowness, still
	     * use origins to group waveforms by net and unique origin.
	     */
	    for(int j = 0; j < wvec.size(); j++)
	    {
	        const char *net = wvec[j]->net();
		if(net != NULL && !recipe->net.compare(net))
		  {
		    origin = wp->getPrimaryOrigin(wvec[j]);

		    if(origin != last_origin)
		    {
			makeBeam(origin, *recipe, selected_only);

			/* need to reread waveform vector as makeBeam might have replaced one Waveform object,
			   not rereading will lead to accessing a Waveform object which has been deallocated */
			if(selected_only) { wp->getSelectedWaveforms(wvec); }
			else { wp->getWaveforms(wvec); }

			last_origin = origin;
			beamed_one = true;
		    }
		}
	    }
	}
	else 
	{
	    /* We need a valid origin for azimuth and/or slowness
	     */
	    for(int j = 0; j < wvec.size(); j++)
	    {
	        const char *net = wvec[j]->net();
		if(net != NULL && !recipe->net.compare(net))
		{
		    found_waveform = true;

		    origin = wp->getPrimaryOrigin(wvec[j]);

		    if(origin != NULL && origin->lon > -900 &&
			origin->lat > -900. && origin != last_origin)
		    {
			makeBeam(origin, *recipe, selected_only);

			/* need to reread waveform vector as makeBeam might have replaced one Waveform object,
			   not rereading will lead to accessing a Waveform object which has been deallocated */
			if(selected_only) { wp->getSelectedWaveforms(wvec); }
			else { wp->getWaveforms(wvec); }

			last_origin = origin;
			beamed_one = true;
		    }
		}
	    }
	    if(found_waveform && last_origin == (CssOriginClass *)-1) {
		showWarning("No origin information for %s",recipe->net.c_str());
	    }
	}
    }
    if(!selected_only) {
	/* only warn if no beams at all were created
	 */
	if(wvec.size() > 0 && !beamed_one) {
	    showWarning("No recipes selected for waveform networks.");
	}
    }
    else
    {
	string last_net;
	/* check if a beam was created for each selected waveform.
	 */
	last_net.assign("");
	for(int j = 0; j < wvec.size(); j++)
	{
	    if(last_net.compare(wvec[j]->net()))
	    {
		int k;
		for(k = 0; k < num_selected; k++) {
		    if(!recipes->at(rows[k]).net.compare(wvec[j]->net())) break;
		}
		if(k == num_selected) {
		    showWarning("No recipes selected for network %s",
				wvec[j]->net());
		}
	    }
	    last_net.assign(wvec[j]->net());
	}
    }
    setCursor("default");
}

void Recipes::makeBeam(CssOriginClass *origin, BeamRecipe &recipe, bool selected_only)
{
    BeamLocation beam_location;
    string beam_name;
    gvector<Waveform *> ws, all_ws;
    vector<double> weights, t_lags;
    double lat, lon, delta, daz, baz, az, slowness;
    IIRFilter *iir = NULL;

    if(!wp) return;

    wp->getWaveforms(all_ws);

    for(int i = 0; i < all_ws.size(); i++) weights.push_back(0.);

    if(selected_only)
    {
	/* Count the number of selected waveforms for this network and
	 * origin. If only one such waveform is selected, use the
	 * beam-group. If more than one waveform is selected, beam with
	 * selected waveforms only.
	 */
	for(int i = 0; i < all_ws.size(); i++)
	{
	    if(all_ws[i]->selected && !recipe.net.compare(all_ws[i]->net()) &&
		origin == wp->getPrimaryOrigin(all_ws[i]))
	    {
		ws.push_back(all_ws[i]);
	    }
	}
    }

    if(!selected_only || ws.size() == 1)
    {
	ws.clear();
	if(getBeamStations(recipe, all_ws, origin, ws, weights) <= 0) return;
	beam_name.assign(recipe.name);
    }
    else if(selected_only)
    {
	gvector<Waveform *> ws_tmp;

	if(getBeamStations(recipe, all_ws, origin, ws_tmp, weights) >0)
	{
	    /* check if all beam stations are selected. */
	    int i, j = 0;
	    for(i = 0; i < ws_tmp.size() && j != ws.size(); i++) {
		for(j = 0; j < ws.size() && ws_tmp[i] != ws[j]; j++);
	    }
	    if(ws_tmp.size() == ws.size() && i == ws_tmp.size()) {
		/* all beam stations were selected.
		 * use weights and beam name
		 */
		ws.clear();
		for(i = 0; i < ws_tmp.size(); i++) ws.push_back(ws_tmp[i]);
		beam_name.assign(recipe.name);
	    }
	    else {
		for(i = 0; i < ws.size(); i++) weights[i] = 1.;
	    }
	}
    }

    if(dnorth_toggle->state()) {
	beam_location = DNORTH_DEAST;
    }
    else if(reference_toggle->state()) {
	beam_location = REFERENCE_STATION;
    }
    else {
	beam_location = ARRAY_CENTER;
    }

    if((beam_location = Beam::getLocation(data_source, beam_location, ws,
		&lon, &lat)) == BEAM_LOCATION_ERROR)
    {
	showWarning(GError::getMessage());
	return;
    }

    if(recipe.azi != -1.) {
	az = recipe.azi;
    }
    else if(origin != NULL) {
	deltaz(origin->lat, origin->lon, lat, lon, &delta, &daz, &baz);
	az = baz;
    }
    else {
	showWarning("compute_beam: origin = NULL.");
	return;
    }
	    
    if(recipe.slow >= 0.) {
	slowness = recipe.slow;
    }
    else if(origin != NULL)
    {
	string op;
	if(!travel_time) {
	    travel_time = new TravelTime();
	}
	double tt = travel_time->getTravelTime(recipe.phase, origin, lat, lon,
		ws[0]->elev(), ws[0]->net(), ws[0]->sta(), &slowness, op);
	if(tt < 0.)
	{
	    showWarning("Cannot compute slowness for phase %s.",
			recipe.phase.c_str());
	    return;
	}
	// convert slowness from sec/deg to sec/km.
	slowness *= 180./(3.14159265*6371.);

    }
    else {
	showWarning("compute_beam: origin = NULL.");
	return;
    }

    if( !Beam::getTimeLags(data_source, ws, az, slowness, beam_location,
				t_lags, &lat, &lon) )
    {
	showWarning(GError::getMessage());
	return;
    }

    WaveformPlot *dest = wp;

    if(beam_window_toggle->state())
    {
	if(!beam_window) {
	    beam_window = new WaveformWindow("Beams", this);
	}
	beam_window->setVisible(true);
	dest = beam_window->wplot;
    }

    iir = new IIRFilter(recipe.ford, recipe.ftype, recipe.flo, recipe.fhi,
			ws[0]->segment(0)->tdel(), recipe.zp);

    iir->addOwner(this);

    wp->computeBeam(ws, t_lags, beam_name, lat, lon, iir, weights,
		replace_beam_toggle->state(), false, dest);

    iir->removeOwner(this);
}

int Recipes::getBeamStations(BeamRecipe &recipe,
		gvector<Waveform *> &all_ws, CssOriginClass *origin,
		gvector<Waveform *> &ws, vector<double> &weights)
{
    int i, j, nsta, n;
    char missing[500];
    vector<BeamSta> beam_sta;

    if(!wp) return 0;

    /* get stations and weights from beam-group
     */
    if((nsta = Beam::getGroup(recipe, beam_sta)) <= 0)
    {
	showWarning("Error getting beam group:\n%s", GError::getMessage());
	return -1;
    }

    ws.clear();
    for(j = 0; j < nsta; j++)
    {
	for(i = 0; i < all_ws.size(); i++)
	{
	    if(!strcasecmp(all_ws[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(all_ws[i]->chan(), beam_sta[j].chan) &&
		   origin == wp->getPrimaryOrigin(all_ws[i]))
		
	    {
		ws.push_back(all_ws[i]);
		break;
	    }
	}
    }
    if(ws.size() == 0) {
	showWarning("No waveforms for beam group %s/%s",
			recipe.net.c_str(), recipe.group.c_str());
	return 0;
    }

    missing[0] = '\0';
    n = 1;
    for(j = 0; j < nsta; j++) if(beam_sta[j].wgt != 0.)
    {
	for(i = 0; i < ws.size(); i++)
	{
	    if( !strcasecmp(ws[i]->sta(), beam_sta[j].sta) &&
		DataSource::compareChan(ws[i]->chan(), beam_sta[j].chan))
	    {
		weights[i] = beam_sta[j].wgt;
		break;
	    }
	}
	if(i == ws.size()) {
	    size_t len = 2 + strlen(beam_sta[j].sta) + 1
				+ strlen(beam_sta[j].chan);
	    if(n + len < sizeof(missing))
	    {
		if(missing[0] != '\0') strcat(missing, ", ");
		strcat(missing, beam_sta[j].sta);
		strcat(missing, "/");
		strcat(missing, beam_sta[j].chan);
		n += len;
	    }
	}
    }
    if(missing[0] != '\0') {
	showWarning("Missing beam channels: %s.", missing);
    }

    return ws.size();
}

static int
sort_by_netorigin(const void *A, const void *B)
{
    Waveform *a = *(Waveform **)A;
    Waveform *b = *(Waveform **)B;
    CssOriginClass *oa=NULL, *ob=NULL;
    register int cnd;

    if( (cnd = strcmp(a->net(), b->net())) ) return (cnd);

    
    oa = (a->cplot) ? a->cplot->getPrimaryOrigin(a) : NULL;
    ob = (b->cplot) ? b->cplot->getPrimaryOrigin(b) : NULL;

    if(oa == ob) return 0;

    return((long)oa - (long)ob);
}

void Recipes::loadRefSta(void)
{
    int num_selected;
    vector<int> rows, nets;

    if(!recipes || !data_source  || !wp || !wp->numWaveforms() ||
		(num_selected = table->getSelectedRows(rows)) <= 0)
    {
	return;
    }

    for(int i = 0; i < num_selected; i++) {
	int j, q_net = stringUpperToQuark(recipes->at(rows[i]).net);
	for(j = 0; j < (int)nets.size() && q_net != nets[j]; j++);
	if(j == (int)nets.size()) {
	    nets.push_back(q_net);
	}
    }

    gvector<Waveform *> wvec;
    wp->getWaveforms(wvec, false);
    if(wvec.size() > 0) {
	DataSource *ds = wvec[0]->getDataSource();

	if(!ref_sta_window) {
	    ref_sta_window = new RefSta("Select Reference Station", this,
					nets, ds);
	}
	else {
	    ref_sta_window->setDataSource(ds, nets);
	}
    }
    ref_sta_window->setVisible(true);
}

void Recipes::deleteRecipe(void)
{
    int nrows;
    vector<int> rows;

    if((nrows = table->getSelectedRows(rows)) <= 0) {
	showWarning("No recipes are selected.");
	return;
    }
    char s[50], s2[20];

    if(nrows == 1) {
	snprintf(s, sizeof(s), "Delete 1 Recipe?");
	strcpy(s2, "Delete Recipe");
    }
    else {
	snprintf(s, sizeof(s), "Delete %d Recipes?", nrows);
	strcpy(s2, "Delete Recipes");
    }
    int ans = Question::askQuestion("Confirm Delete Recipe", this, s, s2,
			"Cancel");
    if(ans == 1) // button number
    {
	for(int i = 0; i < nrows; i++) {
	    if( !Beam::changeRecipe(&recipes->at(rows[i]), true) ) {
		showWarning(GError::getMessage());
		break;
	    }
	}
	readRecipes(true);
	list();
    }
}
