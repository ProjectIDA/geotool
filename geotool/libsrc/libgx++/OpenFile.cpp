/** \file OpenFile.cpp
 *  \brief Defines class OpenFile.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "motif++/MotifClasses.h"
#include "OpenFile.h"
#include "TableQuery.h"
#include "WaveformWindow.h"
#include "TableSource.h"
#include "SacSource.h"
#include "GseSource.h"
#include "AscSource.h"
#include "SeedSource.h"
#include "CSSTable.h"
#include "ImportSeedStation.h"

class Settings : public FormDialog
{
    public:
	Settings(const string &, OpenFile *);
	~Settings(void) {}

	void setVisible(bool visible);
	void actionPerformed(ActionEvent *action_event);
	void apply(void);

	RowColumn *controls, *rc;
	Form *form1, *form2;
	Button *close_button, *apply_button;
	RadioBox *rb;
	Toggle *preview_on_toggle, *preview_off_toggle, *align_toggle;
	Separator *sep;
	Label *preview_label, *label, *num_label, *max_label;
	TextField *num_text, *max_text;

	OpenFile *op;
	int num_waveforms;
	double max_length;
	bool display_preview;
	bool align_first_pt;
};

typedef struct
{
	XmFontList	tag_list;
	XFontStruct	*tag_font;
	XmFontList	arrival_list;
	XFontStruct	*arrival_font;
	XmFontList	axes_list;
	XFontStruct	*axes_font;
} fontData, *fontDataPtr;

#define offset(field)   XtOffset(fontDataPtr, field)
static XtResource resources[] =
{
    {(char *)"previewTagFont", (char *)"PreviewTagFont", XmRFontList,
	sizeof(XmFontList), offset(tag_list), XtRString,
	(XtPointer)"-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*"},
    {(char *)"previewArrivalFont", (char *)"PreviewArrivalFont", XmRFontList,
	sizeof(XmFontList), offset(arrival_list), XtRString,
	(XtPointer)"-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*"},
    {(char *)"previewAxesFont", (char *)"PreviewAxesFont", XmRFontList,
	sizeof(XmFontList), offset(axes_list), XtRString,
	(XtPointer)"-adobe-helvetica-bold-r-*-*-10-*-*-*-*-*-*-*"},
};

static fontData font_data;


OpenFile::OpenFile(const string &name, WaveformWindow *parent,
	FileType filetype, const string &dir, const string &pattern,
	const string &open_button_name) : CssFileDialog(name, parent, filetype,
		dir, pattern, open_button_name), DataReceiver(NULL)
{
    parent_wplot = parent;
    init(pattern);
}

OpenFile::OpenFile(const string &name, WaveformWindow *parent,FileType filetype,
		const string &dir, const string &pattern) :
		CssFileDialog(name, parent, filetype, dir, pattern, "Open"),
		DataReceiver(NULL)
{
    parent_wplot = parent;
    init(pattern);
}

OpenFile::~OpenFile()
{
    Free(table_names);
}

void OpenFile::init(const string &pattern)
{
    seed_source = NULL;
    tq_display_seed = NULL;
    tq_list_records = NULL;
    tq_list_blockettes = NULL;
    tq_blockette_contents = NULL;
    blockette_window = NULL;

    XmFontContext context;
    XmStringCharSet charset;

    Application *application = Application::getApplication();

    XtGetApplicationResources(application->baseWidget(), &font_data, resources,
				XtNumber(resources), NULL, 0);
    if(!XmFontListInitFontContext(&context, font_data.tag_list)
	|| !XmFontListGetNextFont(context, &charset, &font_data.tag_font))
    {
	showWarning("fontsInit: invalid fontList for previewTagFont");
	return;
    }
    XtFree(charset);
    XmFontListFreeFontContext(context);

    if(!XmFontListInitFontContext(&context, font_data.arrival_list)
	|| !XmFontListGetNextFont(context, &charset, &font_data.arrival_font))
    {
	showWarning("fontsInit: invalid fontList for previewArrivalFont");
	return;
    }
    XtFree(charset);
    XmFontListFreeFontContext(context);

    if(!XmFontListInitFontContext(&context, font_data.axes_list)
	|| !XmFontListGetNextFont(context, &charset, &font_data.axes_font))
    {
	showWarning("fontsInit: invalid fontList for previewAxesFont");
	return;
    }
    XtFree(charset);
    XmFontListFreeFontContext(context);

    import_button = new Button("Import Seed...", getControls(), this);
    import_button->setSensitive(false);

    settings_button = new Button("Settings...", getControls(), this);

    import_seed_button = new Button("Import Seed Station...", tq->fileMenu(), 0, this);
    import_seed_button->setVisible(false);
    import_seed_station = new ImportSeedStation("Import Seed Stations", tq, NULL);

    ds = new TableSource("local");
    wplot = NULL;

    num_table_names = CssTableClass::getAllNames(&table_names);
    const char *names[] = {
	"wfdisc", "seed", "asc.gz", "gse.gz", "msg.gz", "sac.gz"};
    setFileSuffixes(6, names, pattern);

    string prop;
    if(getProperty("OpenFile.type", prop)) {
	filter_choice->setChoice(prop);
    }

    settings = new Settings("File Preview Settings", this);

    if(settings->preview_on_toggle->state()) {
	showPreviewArea();
    }
}

void OpenFile::actionPerformed(ActionEvent *action_event)
{
    Component *comp = action_event->getSource();

    if(comp == open_button || comp == list_button)
    {
	putProperty("OpenFile.type", filter_choice->getChoice());
    }

    if(comp == tq_list_records) {
	char *file = getFile(false);
	if(file) {
	    SeedSource *se = new SeedSource("seed_source", file);
	    se->addOwner(this);
	    setCursor("hourglass");
	    se->listRecords();
	    tq->displayTable(se, "Data_Records");
	    tq->tabSetOnTop("Data_Records");
	    se->removeOwner(this);
	    XtFree(file);
	    setCursor("default");
	}
	return;
    }
    else if(comp == tq_list_blockettes) {
	listBlockettes();
	return;
    }
    else if(comp == tq_blockette_contents) {
	setCursor("hourglass");
	showBlockettes();
	setCursor("default");
    }
    else if(comp == tq_display_seed) {
	if(!seed_source) return;
	seed_source->selected.clear();
	seed_source->selected_records.clear();
	CSSTable *css = tq->tableOnTop();
	if(!strcmp(css->getType(), "Waveforms")) {
	    css->getSelectedRows(seed_source->selected);
	}
	else if(!strcmp(css->getType(), "Data_Records")) {
	    css->getSelectedRows(seed_source->selected_records);
 	}
	parent_wplot->setDataSource(seed_source);
	return;
    }
    else if(comp == settings_button) {
	settings->setVisible(true);
	return;
    }
    else if( !strcmp(action_event->getReason(), XtNfileSelectCallback) ) {
	Widget w = (Widget)action_event->getCalldata();
	int nitems=0;
	Arg args[1];
	XtSetArg(args[0], XmNselectedItemCount, &nitems);
	XtGetValues(w, args, 1);
	if(nitems == 1) {
	    char *file = getFile(false);
	    setCursor("hourglass");
	    if(stringCaseEndsWith(file, ".seed") ||
    	    	(!stringCaseEndsWith(file, ".sac") &&
		 !stringCaseEndsWith(file, ".sac.gz") &&
		 !stringCaseEndsWith(file, ".asc") &&
		 !stringCaseEndsWith(file, ".asc.gz") &&
		 !stringCaseEndsWith(file, ".msg") &&
		 !stringCaseEndsWith(file, ".wfdisc") &&
			SeedSource::isSeedFile(file)) )
	    {
		import_button->setSensitive(true);
	    }
	    else {
		import_button->setSensitive(false);
	    }
	    XtFree(file);
	    setCursor("default");
	}
	else {
	    import_button->setSensitive(false);
	}
    }
    else if(comp == import_seed_button || comp == import_button) {
	import_seed_station->setVisible(true);
	import_seed_station->setSeedFile(getFile(false));
	return;
    }
    if(comp == list_button) {
	char *file = getFile(false);
	if(file) {
	    if(stringCaseEndsWith(file, ".seed") ||
    	    	(!stringCaseEndsWith(file, ".sac") &&
		 !stringCaseEndsWith(file, ".sac.gz") &&
		 !stringCaseEndsWith(file, ".asc") &&
		 !stringCaseEndsWith(file, ".asc.gz") &&
		 !stringCaseEndsWith(file, ".msg") &&
		 !stringCaseEndsWith(file, ".wfdisc") &&
			SeedSource::isSeedFile(file)) )
	    {
		if(seed_source != NULL) {
		    seed_source->removeOwner(this);
		}
		seed_source = new SeedSource("seed_source", file);
		seed_source->addOwner(this);
		seed_source->listWaveforms();
		import_seed_button->setVisible(true);
		tq->query_button->setVisible(false);
		tq->save_to_file_button->setVisible(false);
		tq->save_all_button->setVisible(false);
		tq->display_arrivals->setVisible(false);
		tq->display_waveforms->setVisible(false);
		if(!tq_display_seed) {
		    tq_display_seed = new Button("Display Waveforms",
					tq->optionMenu(), this);
		    tq->toolBar()->add(tq_display_seed);
		}
		else {
		    tq_display_seed->setVisible(true);
		}
		if(!tq_list_blockettes) {
		    tq_list_blockettes = new Button("List Blockettes",
					tq->optionMenu(), this);
		    tq->toolBar()->add(tq_list_blockettes);
		}
		else {
		    tq_list_blockettes->setVisible(true);
		}
		if(!tq_blockette_contents) {
		    tq_blockette_contents = new Button("Blockette Contents",
					tq->optionMenu(), this);
		    tq->toolBar()->add(tq_blockette_contents);
		}
		else {
		    tq_blockette_contents->setVisible(true);
		}
		if(!tq_list_records) {
		    tq_list_records = new Button("List Data Records",
					tq->optionMenu(), this);
		    tq->toolBar()->add(tq_list_records);
		}
		else {
		    tq_list_records->setVisible(true);
		}
/*
    char suffix[100];
    char *filename = strdup(file.c_str());
    int n = (int)strlen(filename);

    suffix[0] = '\0';
    // get prefix and suffix
    for(int i = n-1; i >= 0 && filename[i] != '/'; i--) {
        if(filename[i] == '.') {
            stringcpy(suffix, filename+i+1, sizeof(suffix));
            filename[i] = '\0';
            break;
        }
    }
    setTitle(filename);
    Free(filename);
*/

		tq->setTitle(file);
		tq->setVisible(true);
		tq->removeAllTabs();
		tq->displayAllTables(seed_source);
		listBlockettes();
		tq->tabSetOnTop("Waveforms");
		XtFree(file);
		return;
	    }
	    else {
		import_seed_button->setVisible(false);
		if(tq_display_seed) tq_display_seed->setVisible(false);
		if(tq_list_records) tq_list_records->setVisible(false);
		if(tq_list_blockettes) tq_list_blockettes->setVisible(false);
		if(tq_blockette_contents) {
		    tq_blockette_contents->setVisible(false);
		}
		tq->query_button->setVisible(true);
		tq->save_to_file_button->setVisible(true);
		tq->save_all_button->setVisible(true);
		tq->display_arrivals->setVisible(true);
		tq->display_waveforms->setVisible(true);
	    }
	    XtFree(file);
	}
    }
    CssFileDialog::actionPerformed(action_event);
}

void OpenFile::listBlockettes()
{
    char *file = getFile(false);
    if(file) {
	setCursor("hourglass");
	SeedSource *se = new SeedSource("seed_source", file);
	se->addOwner(this);
	se->listBlockettes();
	tq->displayTable(se, "Stations");
	tq->displayTable(se, "Channels");
	tq->displayTable(se, "Units");
	tq->displayTable(se, "Abbreviations");
	tq->displayTable(se, "Comments");
	tq->displayTable(se, "Blockettes");
	tq->tabSetOnTop("Stations");
	se->removeOwner(this);
	XtFree(file);
	setCursor("default");
    }
}

void OpenFile::showPreviewArea(void)
{
    if(!wplot)
    {
	int n;
	Arg args[20];

	form = new Form("form", rc);
	n = 0;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	date_label = new Label("20April2006", form, args, n);
	n = 0;
	XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
	XtSetArg(args[n], XmNbottomWidget, date_label->baseWidget()); n++;
	XtSetArg(args[n], XtNautoYScale, False); n++;
	XtSetArg(args[n], XtNdisplayAmplitudeScale, False); n++;
	XtSetArg(args[n], XtNverticalScroll, False); n++;
	XtSetArg(args[n], XtNhorizontalScroll, False); n++;
	XtSetArg(args[n], XtNaxesFont, font_data.axes_font); n++;
	XtSetArg(args[n], XtNtagFont, font_data.tag_font); n++;
	XtSetArg(args[n], XtNarrivalFont, font_data.arrival_font); n++;
	int data_height = (int)(300./settings->num_waveforms + .5);
	if(data_height < 5) data_height = 5;
	XtSetArg(args[n], XtNdataHeight, data_height); n++;
	XtSetArg(args[n], XtNleftMargin, 4); n++;
	XtSetArg(args[n], XtNwidth, 400); n++;
	XtSetArg(args[n], XtNheight, 380); n++;

	wplot = new WaveformPlot("preview", form, args, n);

	if(settings->align_first_pt) {
	    wplot->alignWaveforms(ALIGN_FIRST_POINT);
	}
	else {
	    wplot->alignWaveforms(ALIGN_TRUE_TIME);
	}

	CPlotArrivalType atype;
	atype.max_display_value = 1.;
	atype.hardcopy_min_height_pix = 2;
	wplot->addArrivalType(&atype);
	wplot->setSensitive(false);
    }
    CssFileDialog::showPreviewArea();
}

bool OpenFile::previewFile(const string &file)
{
    preview_file = file;
    if(file.empty() || !settings->preview_on_toggle->state()) return false;

    const char *f = file.c_str();

    setCursor("hourglass");

    wplot->clear();
    // limit preview to the first 10 waveforms
    int num_waveforms = getProperty("preview_num_waveforms", 10);
    wplot->setWaveformLimit(num_waveforms);
    // limit preview to the first 60 seconds
    double max_length = getProperty("preview_max_length", 60.);
    wplot->setWaveformMaxLength(max_length);

    if( tableSuffix(file) )
    {
	if(!ds->openPrefix(file)) {
	    setCursor("default");
	    return false;
	}
	// could have a GParse limit the number of tables retrieved.
	ds->query("wfdisc select * from wfdisc");
	ds->query("arrival select * from arrival");

	wplot->setDataSource(ds);
	ds->clearTable("all");
    }
    else if(stringCaseEndsWith(f, ".sac")) {
	SacSource *ss = new SacSource("sac_source", file);
	wplot->setDataSource(ss);
    }
    else if(stringCaseEndsWith(f, ".seed")) {
	SeedSource *se = new SeedSource("seed_source", file);
	wplot->setDataSource(se);
    }
    else if(stringCaseEndsWith(f, ".msg") || stringCaseEndsWith(f,".gse"))
    {
	GseSource *gs = new GseSource("gse_source", file);
	wplot->setDataSource(gs);
    }
    else if(stringCaseEndsWith(f, ".asc")) {
	AscSource *asc = new AscSource("asc_source", file);
	wplot->setDataSource(asc);
    }
    else if(SeedSource::isSeedFile(file)) {
	SeedSource *se = new SeedSource("seed_source", file);
	wplot->setDataSource(se);
    }
    else if(AscSource::isAscFile(file)) {
	AscSource *asc = new AscSource("asc_source", file);
	wplot->setDataSource(asc);
    }
    else
    {
	setPrintError(false);
	// try GSE message
	GseSource *gs = new GseSource("gse_source", file);
	wplot->setDataSource(gs);
	if( !wplot->numWaveforms() ) {
	    // try SAC
	    SacSource *ss = new SacSource("sac_source", file);
	    wplot->setDataSource(ss);
	}
	setPrintError(true);
    }

    gvector<Waveform *> wvec;
    wplot->getWaveforms(wvec);
    if(wvec.size() > 0) {
	double tmin = wvec[0]->tbeg();
	for(int i = 1; i < wvec.size(); i++) {
	    double t0 = wvec[i]->tbeg();
	    if(t0 < tmin) tmin = t0;
	}
	char str[100];
	date_label->setLabel(timeEpochToString(tmin, str, sizeof(str), YMOND));
    }
    else {
	date_label->setLabel("---");
    }
    setCursor("default");

    return (wvec.size() > 0) ? true : false;
}

bool OpenFile::tableSuffix(const string &file)
{
    for(int i = 0; i < num_table_names; i++)
    {
	if( stringCaseEndsWith(file.c_str(), table_names[i]) ) return true;
    }
    return false;
}

void OpenFile::showBlockettes()
{
    char *file, s[200];
    Seed *o;
    CSSTable *css = tq->tableOnTop();

    if( !css || !(file = getFile(false)) ) return;

    ifstream ifs;
    ifs.open(file);
    if( !ifs.good() ) {
        char error[MAXPATHLEN+100];
        snprintf(error, sizeof(error), "seed: cannot open %s", file);
        logErrorMsg(LOG_WARNING, error);
	XtFree(file);
        return;
    }
    XtFree(file);

    if(!blockette_window) {
	XmFontList list = XmFontListCreate(css->getFont(),
				XmSTRING_DEFAULT_CHARSET);
	blockette_window = new TextDialog("Blockette Contents", tq);
	Arg args[2];
	XtSetArg(args[0], XtNbackground,
		blockette_window->stringToPixel("white"));
	XtSetArg(args[1], XmNfontList, list);
	blockette_window->textField()->setValues(args, 2);
    }
    blockette_window->setText("");

    SeedInput in(&ifs);

    blockette_window->textField()->disableRedisplay();

    while( (o = in.readSeed()) ) {
	if( !o->getSeedData() ) {
	    snprintf(s, sizeof(s), "%6d %c B%s\n", in.recordSeqenceNumber(),
		(char)in.recordType(), o->getType().c_str());
	    blockette_window->append(s);
	    blockette_window->append(o->longStr());
	    blockette_window->append("\n\n");
	}
	delete o;
    }
    
    blockette_window->textField()->enableRedisplay();
    blockette_window->textField()->showPosition(0);
    blockette_window->setVisible(true);
}

Settings::Settings(const string &name, OpenFile *parent)
		: FormDialog(name, parent, false, false)
{
    int n;
    Arg args[20];

    op = parent;
    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    controls = new RowColumn("controls", this, args, n);

    apply_button = new Button("Apply", controls, this);
    apply_button->setSensitive(false);
    close_button = new Button("Close", controls, this);

    n = 0;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("File Preview Settings", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 10); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rc = new RowColumn("rc", this, args, n);

    preview_label = new Label("Display File Preview", rc);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    rb = new RadioBox("rb", rc, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, True); n++;
    preview_on_toggle = new Toggle("On", rb, this, args, n);

    n = 0;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNset, False); n++;
    preview_off_toggle = new Toggle("Off", rb, this, args, n);

    display_preview = getProperty("preview_display", true);
    preview_on_toggle->set(display_preview, false);
    preview_off_toggle->set(!display_preview, false);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, rc->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 7); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    form1 = new Form("form1", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    num_text = new TextField("num_text", form1, this, args, n);
    num_waveforms = getProperty("preview_num_waveforms", 10);
    num_text->setString(num_waveforms);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, num_text->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
    num_label = new Label("Number of Preview Waveforms", form1, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form1->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 2); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 7); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    form2 = new Form("form2", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNcolumns, 8); n++;
    max_text = new TextField("max_text", form2, this, args, n);
    max_length = getProperty("preview_max_length", 60.);
    max_text->setString("%.5g", max_length);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNrightWidget, max_text->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNalignment, XmALIGNMENT_CENTER); n++;
    max_label = new Label("Max Preview Length (secs)", form2);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, form2->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 8); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 4); n++;
    align_toggle = new Toggle("Align on first point", this, this, args, n);
    align_first_pt = getProperty("preview_align_first_pt", true);
    align_toggle->set(align_first_pt, false);
}

void Settings::setVisible(bool visible)
{
    if(visible) {
	preview_on_toggle->set(display_preview, false);
	preview_off_toggle->set(!display_preview, false);
	align_toggle->set(align_first_pt, false);
	num_text->setString(num_waveforms, false);
	max_text->setString("%.5g", max_length, false);
    }
    FormDialog::setVisible(visible);
}

void Settings::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();
    Component *comp = action_event->getSource();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Apply")) {
	apply();
	apply_button->setSensitive(false);
    }
    else if(comp == preview_on_toggle || comp == preview_off_toggle ||
	comp == num_text || comp == max_text || comp == align_toggle)
    {
	int i;
	double d;
	bool set = false;
	if(display_preview != preview_on_toggle->state()) set = true;
	if(!num_text->getInt(&i) || i != num_waveforms) set = true;
	if(!max_text->getDouble(&d) || d != max_length) set = true;
	if(align_first_pt != align_toggle->state()) set = true;
	apply_button->setSensitive(set);
    }
}

void Settings::apply(void)
{
    int num;
    ostringstream os;
    max_text->getDouble(&max_length);
    num_text->getInt(&num);
    if(num != num_waveforms) {
	num_waveforms = num;
	Arg args[1];
	int data_height = (int)(300./num_waveforms + .5);
	if(data_height < 5) data_height = 5;
	if(data_height > 60) data_height = 60;
	XtSetArg(args[0], XtNdataHeight, data_height);
	op->wplot->setValues(args, 1);
    }
    if(align_toggle->state() != align_first_pt) {
	align_first_pt = align_toggle->state();
	if(align_first_pt) {
	    op->wplot->alignWaveforms(ALIGN_FIRST_POINT);
	}
	else {
	    op->wplot->alignWaveforms(ALIGN_TRUE_TIME);
	}
    }
    if(display_preview != preview_on_toggle->state()) {
	display_preview = preview_on_toggle->state();
	if(display_preview) op->showPreviewArea();
	else op->hidePreviewArea();
    }

    op->previewFile(op->preview_file);

    os.precision(2);
    os << num_waveforms;
    putProperty("preview_num_waveforms", os.str());
    os.str("");
    os << max_length;
    putProperty("preview_max_length", os.str());
    putProperty("preview_align_first_pt", align_first_pt ? "true" : "false");
    putProperty("preview_display", display_preview ? "true" : "false");
}
