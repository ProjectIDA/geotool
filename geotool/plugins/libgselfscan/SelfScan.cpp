/** \file SelfScan.cpp
 *  \brief Defines class SelfScan.
 *  \author Vera Miljanovic
 */

#include "config.h"
#include <iostream>
#include <math.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

#include "SelfScan.h"
#include "SelfScanParams.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "Waveform.h"

extern "C" {
#include "cluster.h"
}


/* Self Scanning Algorithm (see Exploring the limits of waveform correlation 
 * event detection as applied to the 1994 Northridge earthquake aftershock
 * sequence by Carr, Resor, Young, Procopio 2009)
*/


using namespace libgselfscan;

SelfScan::SelfScan(const char *name, Component *parent,
		   DataSource *ds) : Frame(name, parent, true), DataReceiver(ds)
{
    num_selfscan = 0;
    selfscan= NULL;
    num_clusters = 0;
    clusterids = NULL;

    /* first (only) argument is clusterid */
    cluster_title_format = strdup("Cluster %d");

    createInterface();
}

void SelfScan::createInterface()
{
    int n;
    Arg args[30];

    setSize(1200, 600);

    menu_bar = new MenuBar("menuBar", this);
    tool_bar = new ToolBar("toolbar", this, menu_bar);
    info_area = new InfoArea("infoArea", this);

    file_menu = new Menu("File", menu_bar);
    print_button = new Button("Print...", file_menu, this);
    close_button = new Button("Close", file_menu, this);

    edit_menu = new Menu("Edit", menu_bar);
    clear_button = new Button("Clear", edit_menu, this);
    delete_button = new Button("Delete", edit_menu, this);

    view_menu = new Menu("View", menu_bar);
    attributes_button = new Button("Attributes...", view_menu, this);
  
    option_menu = new Menu("Option", menu_bar);
    parameters_button = new Button("Parameters...", option_menu, this);
    align_button = new Button("Align Waveforms", option_menu, this);
    copy_button = new Button("Copy to Main Window", option_menu, this);
    cluster_s_button = new Button("Single-Linkage", option_menu, this);
    cluster_m_button = new Button("Maximum-Linkage", option_menu, this);
    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Self Scanning Help",
			     help_menu, this);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    pane = new Pane("pane", frame_form, args, n);

    n = 0;
    XtSetArg(args[n], XtNvisibleRows, 10); n++;
    XtSetArg(args[n], XmNbackground, stringToPixel("white")); n++;
    XtSetArg(args[n], XmNforeground, stringToPixel("black")); n++;
    XtSetArg(args[n], XtNeditable, False); n++;
    XtSetArg(args[n], XtNselectable, True); n++;
    XtSetArg(args[n], XtNcolumns, 7); n++;
    const char *col_labels[] = {"cluster", "sta", "chan", "max coef", "time", "abs corr lag", "rel corr lag"};
    XtSetArg(args[n], XtNcolumnLabels, col_labels); n++;
    table = new Table("table", pane, args, n);
    table->setAttributes("cluster,%d,sta,%s,chan,%s,max coef,%.8f,time,%t,abs corr lag,%.1f,rel corr lag,%.1f");
    int alignment[8] = {LEFT_JUSTIFY, LEFT_JUSTIFY, LEFT_JUSTIFY,
			RIGHT_JUSTIFY, LEFT_JUSTIFY, RIGHT_JUSTIFY,
			RIGHT_JUSTIFY};
    table->setAlignment(7, alignment);
    table->addActionListener(this, XtNselectRowCallback);
    table->addActionListener(this, XtNattributeChangeCallback);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopOffset, 4); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    tab = new TabClass("tab", pane, args, n);
    tab->addActionListener(this, XtNtabCallback);

    plot = NULL;

    param_window = new SelfScanParams("Self Scanning Parameters", this, this);
    print_window = NULL;

    setButtonsSensitive();

    addPlugins("Self Scanning Correlation", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	//tool_bar->add(parameters_button, "Parameters...");
	tool_bar->add(cluster_s_button, "Single-Linkage");
	tool_bar->add(cluster_m_button, "Maximum-Linkage");
    }
}

SelfScan::~SelfScan(void)
{
  clear();
}

void SelfScan::actionPerformed(ActionEvent *a)
{
    const char *cmd = a->getActionCommand();
    const char *reason = a->getReason();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Print...")) {
	print();
    }
    else if(!strcmp(cmd, "Clear")) {
	clear();
    }
    else if(!strcmp(cmd, "Delete")) {
	remove();
    }
    else if(!strcmp(cmd, "Parameters...")) {
	param_window->setVisible(true);
    }
    else if(!strcmp(cmd, "Single-Linkage")) {
	clear();
	compute('s');
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Maximum-Linkage")) {
	clear();
	compute('m');
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Align Waveforms")) {
	align();
    }
    else if(!strcmp(cmd, "Copy to Main Window")) {
	copyWaveform();
    }
    else if(!strcmp(cmd, "Self Scanning Help")) {
	showHelp(cmd);
    }
    else if(!strcmp(reason, XtNattributeChangeCallback)) {
	list();
	selectWaveform();
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNselectRowCallback)) { // select row
	selectRow();
	setButtonsSensitive();
    }
    else if(!strcmp(reason, XtNselectDataCallback)) { // select waveform
	selectWaveform();
	setButtonsSensitive();
    }
}

ParseCmd SelfScan::parseCmd(const string &cmd, string &msg)
{
    string c;

    if(parseString(cmd, "all", c) && num_clusters > 0) {
        return plot[0]->parseCmd(c, msg);
    }
    for(int i = 0; i < num_clusters; i++) {
	ostringstream os;
	os << "cluster" << i;
	if(parseString(cmd, os.str(), c)) {
	    return plot[i+1]->parseCmd(c, msg);
	}
    }
    if(parseCompare(cmd, "Clear")) {
	clear_button->activate();
    }
    else if(parseCompare(cmd, "Delete")) {
	delete_button->activate();
    }
    else if(parseCompare(cmd, "Copy_to_Main_Window")) {
	copy_button->activate();
    }
    else if(parseCompare(cmd, "Single_Linkage")) {
	cluster_s_button->activate();
    }
    else if(parseCompare(cmd, "Maximum_Linkage")) {
	cluster_m_button->activate();
    }
    else if(parseCompare(cmd, "Align_Waveforms")) {
	align_button->activate();
    }
    else if(parseString(cmd, "print_window", c)) {
	if(!print_window) {
	    print_window = new PrintDialog("Print Cluster", this, this);
	}
	return print_window->parseCmd(c, msg);
    }
    else if(parseString(cmd, "table", c)) {
	return table->parseCmd(c, msg);
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

ParseVar SelfScan::parseVar(const string &name, string &value)
{
    string c;

    if(parseString(name, "table", c)) {
        return table->parseVar(c, value);
    }
    else if(parseString(name, "all", c) && num_clusters > 0) {
        return plot[0]->parseVar(c, value);
    }

    for(int i = 0; i < num_clusters; i++) {
	ostringstream os;
	os << "cluster" << i;
	if(parseString(name, os.str(), c)) {
	    return plot[i+1]->parseVar(c, value);
	}
    }
    return Frame::parseVar(name, value);
}

void SelfScan::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scluster\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%scopy_to_main_window\n", prefix);
    printf("%scluster_s\n", prefix);
    printf("%scluster_m\n", prefix);
    printf("%salign waveforms\n", prefix);
    printf("%sprint.help\n", prefix);
    Table::parseHelp(prefix);
}

bool SelfScan::getParam(struct SelfScanParam_s *cp)
{
    if(!param_window->getDouble("window length in seconds",
				&cp->windowlength)) {
      showWarning("Invalid number of clusters");
      return false;
    }
    if(!param_window->getInt("number of clusters", &cp->nclusters)) {
      showWarning("Invalid number of clusters");
      return false;
    }
    if(!param_window->getDouble("minimum correlation threshold",
				&cp->min_corr_th)) {
      showWarning("Invalid minimum correlation threshold value");
      return false;
    }

    /* Check sanity on windowlength value */
    if (cp->windowlength <= 0.0) {
      showWarning("Window length must be larger than zero seconds.");
      return false;
    }

    /* Check sanity on ncluster value */
    if (cp->nclusters <= 0) {
      showWarning("Number of clusters must be larger than zero.");
      return false;
    }

    /* Check sanity on min_corr_th value */
    if (cp->min_corr_th < (double)0.0 || cp->min_corr_th > (double)1.0) {
      showWarning("Minimum correlation threshold must be between (inclusive) 0 and 1.");
      return false;
    }



    return true;
}

void SelfScan::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Self Scanning", this, this);
    }
    print_window->setVisible(true);
}

void SelfScan::print(FILE *fp, PrintParam *p)
{
    p->top_title = NULL;
    p->x_axis_label = (char *)"time (seconds)";
    AxesParm *a = NULL;

    /* print only current visible waveform plot (tab) */
    char *ontop = tab->labelOnTop();
    if (strcmp(ontop, "All") == 0) {
      a = plot[0]->hardCopy(fp, p, NULL);
    }
    else {
      for (int i = 0; i < num_clusters; ++i) {
	char *title = getClusterTitle(i + 1);
	if (strcmp(ontop, title) == 0) {
	  a = plot[i + 1]->hardCopy(fp, p, NULL);
	  delete [] title;
	  break;
	}
	delete [] title;
      }
    }

    Free(a);
}

void SelfScan::compute(const char cluster_method)
{
    SelfScanParam cp = SELFSCAN_PARAM_NULL;
    int select_mode = 0;
    gvector<Waveform *> wvec;
    GTimeSeries *ts = NULL, *ts_red = NULL, *ts_green = NULL, *ts_segment = NULL;
    int num_segments = 0;
    double *location = NULL;
    double **distmatrix = NULL, *distmatrix_buffer = NULL;
    double *max_coefs = NULL;
    Node *tree = NULL;
    Arg args[30];

    if (data_source == NULL) {
      showWarning("No data.");
      return;
    }

    if( !getParam(&cp) ) return;

    if( data_source->getSelectedWaveforms("a", wvec) < 1) {
      data_source->getSelectedWaveforms(wvec);
      select_mode = 2;
    }
    else {
      select_mode = 1;
    }
    if (wvec.size() < 1) {
      showWarning("Less than 1 waveform selected.");
      return;
    }
    else if (wvec.size() > 1) {
      showWarning("More than 1 waveform selected.");
      return;
    }

    /* Pick out timeseries from selected waveform */
    if (select_mode == 1) {
      double tbeg = wvec[0]->dw[0].d1->time();
      double tend = wvec[0]->dw[0].d2->time();
      ts = wvec[0]->ts->subseries(tbeg, tend);
    }
    else if (select_mode == 2) {
      ts = wvec[0]->ts;
    }

    if (ts->duration() <= cp.windowlength) {
      showWarning("Selected waveform (%f seconds) must be longer than window length (%f seconds).",
		  ts->duration(), cp.windowlength);
      return;
    }

    /* Determine maximum number of red and green windows/segments */
    int max_segments = (ts->length() / 1) + /* green */
      (int)(ts->duration() / (cp.windowlength/2)); /* red */
    
    if (max_segments > MAX_SEGMENTS) {
      max_segments = MAX_SEGMENTS;
      showWarning("Limiting maximum segments to %d due to memory requirements. Increase window size or shorten waveform selection.",
		  max_segments);
    }

    /* Allocate selfscan structure */
    if(!selfscan) selfscan = (SelfScanStruct *)mallocWarn(sizeof(SelfScanStruct));
    selfscan = (SelfScanStruct *)reallocWarn(selfscan, (num_selfscan+max_segments) *  sizeof(SelfScanStruct));

    /* Allocate and initialize location vector for matching data segments */
    location = new double[max_segments];

    /* Allocate and initialize distmatrix for clustering */
    distmatrix_buffer = new double[max_segments * max_segments];
    distmatrix = new double *[max_segments];
    for (int i = 0; i < max_segments; i++) {
      distmatrix[i] = distmatrix_buffer + i * max_segments; 
    }
    for (int i = 0; i < max_segments; i++) {
      for (int j = 0; j < max_segments; j++) {
	distmatrix[i][j] = 0.0;
      }
    }

    /* Allocate and initialize maximum coefficient vector */
    max_coefs = new double[max_segments];
    for (int i = 0; i < max_segments; ++i) {
      max_coefs[i] = -999.9; /* Initially assume small wrong value */
    }

    setCursor("hourglass");

    /* Locate matching data segments */
    for (double t = ts->tbeg();
	 t + cp.windowlength <= ts->tend();
	 t += cp.windowlength/2 /* red window length / 2 */
	 ) {

      if (num_segments >= max_segments) break;

      ts_red = ts->subseries(t, t + cp.windowlength);

      if (ts_red == NULL) continue;

      float *data_red = new float[ts_red->length()];
      double sigma_red = 0.0;

      ts_red->copyInto(data_red);

      for (int i = 0; i < ts_red->length(); i++) {
	sigma_red += pow(data_red[i], 2);
      }
      sigma_red = sqrt(sigma_red);

      for (double t2 = t; /* red window start time */
	   t2 + cp.windowlength <= ts->tend();
	   t2 += (1/((double)ts->length()/ts->duration())) /* 1 sample */
	   ) {

	if (num_segments >= max_segments) break;

	ts_green = ts->subseries(t2, t2 + cp.windowlength);

	if (ts_green == NULL) continue;

	float *data_green = new float[ts_green->length()];
	double sigma_green = 0.0;
	double coef = 0.0;
	int n = ts_red->length();

	ts_green->copyInto(data_green);

	for (int i = 0; i < ts_green->length(); i++) {
	  sigma_green += pow(data_green[i], 2);
	}
	sigma_green = sqrt(sigma_green);

	/* Calculate correlation coefficient */
	for (int i = 0; i < n; i++) {
	  coef += data_red[i] * data_green[i];
	}
	coef = coef / (sigma_red * sigma_green);
	
	/* if correlation coefficient is larger than threshold */
	if ((coef > cp.min_corr_th) && (t != t2)) {

	  int t_index, t2_index;

	  /* Find location t in location vector or first free spot */
	  /* Start at 1 to avoid diagonal in symmetric matrix */
	  for (t_index = 1; t_index < num_segments; t_index++) {
	    if (t == location[t_index]) break;
	  }
	  if (t_index == num_segments) {
	    num_segments++;
	  }
	  if (t != location[t_index]) {
	    location[t_index] = t;
	  }

	  /* Find location t2 in location vector or first free spot */
	  /* Fill lower left part of symmetric matrix */
	  for (t2_index = 0; t2_index < num_segments; t2_index++) {
	    if (t2 == location[t2_index]) break;
	  }
	  if (t2_index == num_segments) {
	    num_segments++;
	  }
	  if (t2 != location[t2_index]) {
	    location[t2_index] = t2;
	  }

	  int x, y;

	  /* Determine distmatrix coordinates */
	  if (t_index < t2_index) {
	    x = t2_index;
	    y = t_index;
	  }
	  else {
	    x = t_index;
	    y = t2_index;
	  }

	  /* Fill in distmatrix */
	  distmatrix[x][y] = coef;

	  /* Save maximum coefficient */
	  if (coef > max_coefs[t_index]) max_coefs[t_index] = coef;
	  if (coef > max_coefs[t2_index]) max_coefs[t2_index] = coef;

	  /*
	  printf("DEBUG: red %f %d matches green %f %d with coef = %f, x = %d, y = %d\n", t, t_index, t2, t2_index, coef, x, y);
	  */
	  
	}

	delete [] data_green;
	ts_green->deleteObject();
	
      }

      delete [] data_red;
      ts_red->deleteObject();

    }

    setCursor("default");

    if (num_segments < cp.nclusters) {
      showWarning("Only had %d matching segments, need at least %d segments for %d clusters.",
		  num_segments, cp.nclusters, cp.nclusters);
    }
    else {
      tree = treecluster(num_segments,
			 num_segments,
			 NULL,            /* not used if distmatrix given */
			 NULL,            /* not used if distmatrix given */
			 NULL,            /* not used if distmatrix given */
			 0,               /* transpose */
			 (char)0,         /* not used if distmatrix given */
			 cluster_method,
			 distmatrix);

      if (tree == NULL) {
	showWarning("Failed to perform hierarchical clustering. See terminal for details. ");
      }
      else {
	clusterids = new int[num_segments];

	cuttree(num_segments, tree, cp.nclusters, clusterids);

	/*
	for (int i = 0; i < num_segments; ++i) {
	  printf("DEBUG: segment %d belongs to cluster %d\n",
		 i, clusterids[i]);
	}
	*/

	clear_clusters();

	plot = new WaveformView *[cp.nclusters + 1];

	if (plot == NULL) {
	  showWarning("Failed to allocate memory.");
	}
	else {
	  num_clusters = cp.nclusters;

	  int n;

	  n = 0;
	  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
	  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XmNshadowThickness, 2); n++;

	  n = 0;
	  XtSetArg(args[n], XtNtimeScale, TIME_SCALE_SECONDS); n++;
	  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
	  XtSetArg(args[n], XtNtitle, "All"); n++;
	  plot[0] = new WaveformView("All", tab, args, n);
	  plot[0]->addActionListener(this, XtNselectDataCallback);
	  for (int i = 0; i < num_clusters; ++i) {
	    char *title = getClusterTitle(i);
	    n--; XtSetArg(args[n], XtNtitle, title); n++;
	    plot[i + 1] = new WaveformView(title, tab, args, n);
	    delete [] title;
	    plot[i + 1]->addActionListener(this, XtNselectDataCallback);
	  }

	  double duration_all = 0.0;
	  double *duration_cluster = new double[num_clusters];
	  for (int i = 0; i < num_clusters; ++i) {
	    duration_cluster[i] = 0.0;
	  }

	  for (int i = 0; i < num_segments; i++) {
	    ts_segment = ts->subseries(location[i],
				       location[i] + cp.windowlength);
	    
	    ts_segment->putValue("max_coef", max_coefs[i]);
	    ts_segment->putValue("segment_id", i);
	    ts_segment->putValue("cluster_id", clusterids[i]);
	    
	    clusterAdd(wvec[0], ts_segment, clusterids[i]);
	    
	    double d = ts_segment->tend() - ts_segment->tbeg();
	    if (duration_all < d) {
	      duration_all = d;
	    }
	    if (duration_cluster[clusterids[i]] < d) {
	      duration_cluster[clusterids[i]] = d;
	    }

	    /* ts_segment->deleteObject(); */
	  }

	  delete [] duration_cluster;

	  /* adjust table columns */
	  table->adjustColumns();
	}
	
	delete [] clusterids;
      }

      Free(tree);
    }

    delete [] distmatrix_buffer;
    delete [] distmatrix;
    delete [] location;
    
    if (select_mode == 1) {
      ts->deleteObject();
    }

    tab->setOnTop("All");
}

void SelfScan::clusterAdd(Waveform *cd, GTimeSeries *selfscan_ts, int clusterid)
{
    char clusterid_string[20], max_coef[20], time[50], absCorrLag[50],
      relCorrLag[50], chan[20];
    GDataPoint *dmin, *dmax, *dp;
    
    selfscan_ts->setSta(cd->sta());
    snprintf(chan, sizeof(chan),"%sC", cd->chan());
    selfscan_ts->setChan(chan);
    selfscan_ts->setNet(cd->net());
    plot[0]->addWaveform(selfscan_ts, cd->fg);
    plot[clusterid + 1]->addWaveform(selfscan_ts, cd->fg);

    dmin = selfscan_ts->minPoint();
    dmax = selfscan_ts->maxPoint();

    dp = (fabs(dmin->value()) > fabs(dmax->value())) ? dmin : dmax;

    strcpy(selfscan[num_selfscan].sta, cd->sta());
    strcpy(selfscan[num_selfscan].chan, cd->chan());
    selfscan_ts->getValue("max_coef", &selfscan[num_selfscan].max_coef);
    selfscan[num_selfscan].time = dp->time();
    selfscan[num_selfscan].abs_corr_lag = dp->time();
    selfscan[num_selfscan].rel_corr_lag = dp->time() - selfscan[0].time;

    int num_columns = table->numColumns();
    const char **row = new const char *[num_columns];

    for(int i = 0; i < num_columns; i++)
    {
	TAttribute a = table->getAttribute(i);
	if(!strcmp(a.name, "cluster")) {
	    snprintf(clusterid_string, sizeof(clusterid_string), a.format,
		     clusterid);
	    row[i] = clusterid_string;
	}
	else if(!strcmp(a.name, "sta")) {
	    row[i] = selfscan[num_selfscan].sta;
	}
	else if(!strcmp(a.name, "chan")) {
	    row[i] = selfscan[num_selfscan].chan;
	}
	else if(!strcmp(a.name, "max coef")) {
	    snprintf(max_coef, sizeof(max_coef), a.format, selfscan[num_selfscan].max_coef);
	    row[i] = max_coef;
	}
	else if(!strcmp(a.name, "time")) {
	    if(!strcmp(a.format, "%t")) {
		row[i] = timeEpochToString(selfscan[num_selfscan].time, time,
					   sizeof(time),
					   YMONDHMS);
	    }
	    else {
		snprintf(time, sizeof(time), a.format, selfscan[num_selfscan].time);
	    }
	}
	else if (!strcmp(a.name, "abs corr lag")) {
	    snprintf(absCorrLag, sizeof(absCorrLag), a.format,
		     selfscan[num_selfscan].abs_corr_lag);
	    row[i] = absCorrLag;
	}
	else if (!strcmp(a.name, "rel corr lag")) {
	    snprintf(relCorrLag, sizeof(relCorrLag), a.format,
		     selfscan[num_selfscan].rel_corr_lag);
	    row[i] = relCorrLag;
	}

    }

    selfscan_ts->putValue("selfscan_max_coef", selfscan[num_selfscan].max_coef);
    selfscan_ts->putValue("selfscan_time", selfscan[num_selfscan].time);
    selfscan_ts->putValue("selfscan_abs_corr_lag", selfscan[num_selfscan].abs_corr_lag);
    selfscan_ts->putValue("selfscan_abs_corr_lag", selfscan[num_selfscan].rel_corr_lag);
    dmin->deleteObject();
    dmax->deleteObject();

    table->addRow(row, false);

    delete [] row;

    num_selfscan++;
}

void SelfScan::list(void)
{
    char max_coef[20], time[50], timeLag[50];
    double t0 = selfscan[0].time;

    table->removeAllRows();
    int num_columns = table->numColumns();
    const char **row = new const char *[num_columns];

    for(int j = 0; j < num_selfscan; j++) {
	for(int i = 0; i < num_columns; i++)
	{
	    TAttribute a = table->getAttribute(i);
	    if(!strcmp(a.name, "sta")) {
		row[i] = selfscan[j].sta;
	    }
	    else if(!strcmp(a.name, "chan")) {
		row[i] = selfscan[j].chan;
	    }
	    else if(!strcmp(a.name, "max coef")) {
		snprintf(max_coef, sizeof(max_coef), a.format, selfscan[j].max_coef);
		row[i] = max_coef;
	    }
	    else if(!strcmp(a.name, "time")) {
		if(!strcmp(a.format, "%t")) {
		    row[i] = timeEpochToString(selfscan[j].time, time, sizeof(time),
					YMONDHMS);
		}
		else {
		    snprintf(time, sizeof(time), a.format, selfscan[j].time);
		}
	    }
	    else {
		snprintf(timeLag, sizeof(timeLag), a.format,
				selfscan[j].time - t0);
		row[i] = timeLag;
	    }
	}
	table->addRow(row, false);
    }
    delete row;
    table->adjustColumns();
}

void SelfScan::setButtonsSensitive(void)
{
  bool sensitive = (table->numRows() > 0) ? true : false;
  align_button->setSensitive(sensitive);
  clear_button->setSensitive(sensitive);
  
  sensitive = table->rowSelected();
  copy_button->setSensitive(sensitive);
  delete_button->setSensitive(sensitive);
}

void SelfScan::copyWaveform(void)
{
    GTimeSeries **selfscan_ts = NULL;
    gvector<Waveform *> wvec;
    Pixel *pixels = NULL;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    /* just using plot[0] (All) works as it is always updated with what is
     * selected elsewhere as well
     */
    if( !plot[0]->getSelectedWaveforms(wvec) ) {
	showWarning("No cluster waveforms selected.");
	return;
    }

    selfscan_ts = new GTimeSeries * [wvec.size()];
    pixels = new Pixel[wvec.size()];

    for(int i = 0; i < wvec.size(); i++) {
	selfscan_ts[i] = new GTimeSeries(wvec[i]->ts);
	pixels[i] = wvec[i]->fg;
    }
    wp->addWaveforms(wvec.size(), selfscan_ts, NULL, pixels);

    delete [] pixels;
    delete [] selfscan_ts;
}

void SelfScan::clear_clusters(void)
{
  table->removeAllRows();
  tab->deleteAllTabs();

  if (plot != NULL) {
/*
    for (int i = 0; i < num_clusters + 1; i++) {
      if (plot[i] != NULL) {
	plot[i]->destroy();
      }
    }
*/
    delete [] plot; plot = NULL;
  }

  num_clusters = 0;
}

void SelfScan::clear(void)
{
  setCursor("hourglass");

  clear_clusters();

  num_selfscan = 0;
  Free(selfscan); selfscan = NULL;

  setButtonsSensitive();

  setCursor("default");
}

void SelfScan::remove(void)
{
    int n, *rows = NULL;
    gvector<Waveform *> wvec, wv, wdel;

    /* just using plot[0] (All) works as it is always updated with what is
     * selected elsewhere as well
     */
    if( !plot[0]->getSelectedWaveforms(wvec) ) {
	showWarning("No cluster waveforms selected.");
	return;
    }

    setCursor("hourglass");

    /* delete the selected waveforms from the table */
    plot[0]->getWaveforms(wv);
    rows = (int *)mallocWarn(wv.size()*sizeof(int));
    n = 0;
    for (int i = 0; i < wv.size(); i++) {
      int j;
      for (j = 0; j < wvec.size() && wv[i] != wvec[j]; j++);

      if(j < wvec.size()) {
	rows[n++] = i;
      }
    }
    table->removeRows(rows, n);

    /* delete selected waveforms from all plots */
    for (int i = num_clusters; i >= 0; --i) {  /* touch plot[0] last */
      plot[i]->getWaveforms(wv);

      for (int j = 0; j < wv.size(); ++j) {
	int cds_id = -1, cd_list_id = -1, k;
	wv[j]->ts->getValue("segment_id", &cds_id);
	//printf("DEBUG: cds_id = %d\n", cds_id);
	for(k = 0; k < wvec.size()
	      && wvec[k]->ts->getValue("segment_id", &cd_list_id)
	      //&& printf("DEBUG: cd_list_id = %d\n", cd_list_id)
	      && cds_id != cd_list_id; k++);

	if (k < wvec.size()) {
	  wdel.push_back(wv[j]);
	}
      }

       plot[i]->deleteWaveforms(wdel);

    }

    /* remove waveforms from selfscan array */
    for(int i = num_selfscan-1; i >= 0; i--) {
      int j;
      for(j = 0; j < n && rows[j] != i; j++);

      if(j < n) {
	// delete row i;
	for(j = i; j < num_selfscan-1; j++) {
	  selfscan[j] = selfscan[j+1];
	}
	num_selfscan--;
      }
    }

    Free(rows);

    setCursor("default");
}

void SelfScan::selectWaveform(void)
{
    vector<bool> states;
    gvector<Waveform *> wvec, wsel;

    /* Maximum number of waveforms/states */
    if(plot[0]->getWaveforms(wvec) <= 0) return;

    setCursor("hourglass");

    /* work on only current visible waveform plot (tab) */
    char *ontop = tab->labelOnTop();
    //printf("DEBUG: in tab: %s\n", ontop);
    if (strcmp(ontop, "All") == 0) {
      plot[0]->getSelectedWaveforms(wsel);
    }
    else {
      for (int i = 0; i < num_clusters; ++i) {
	char *title = getClusterTitle(i);
	if (strcmp(ontop, title) == 0) {
	  plot[i + 1]->getSelectedWaveforms(wsel);
	  delete [] title;
	  break;
	}
	delete [] title;
      }
    }
    //printf("DEBUG: num_selected: %d\n", num_selected);

    /* prepare new row state array for the table (and plots) update */
    for(int i = 0; i < wvec.size(); i++) {
      int cds_id = -1, cd_list_id = -1, j;
      /* cannot compare cds w/ cd_list as CPlotDataClass object is a duplicate
	 for(j = 0; j < num_selected && cds[i] != cd_list[j]; j++);
      */
      wvec[i]->ts->getValue("segment_id", &cds_id);
      //printf("DEBUG: cds_id = %d\n", cds_id);
      for(j = 0; j < wsel.size()
	    && wsel[j]->ts->getValue("segment_id", &cd_list_id)
	    //&& printf("DEBUG: cd_list_id = %d\n", cd_list_id)
	    && cds_id != cd_list_id; j++);

      states.push_back(j < wsel.size() ? true : false);

      /* update all plots with the new select state  */
      for (int k = 0; k < num_clusters + 1; ++k) {
	gvector<Waveform *> wk;
	plot[k]->getWaveforms(wk);

	/* find waveform in each plot corresponding to current waveform in All
	   plot */
	for (int l = 0; l < wk.size(); ++l) {
	  int plot_cd_list_id = -1;
	  wk[l]->ts->getValue("segment_id", &plot_cd_list_id);

	  if (cds_id == plot_cd_list_id) {
	    /* set new state for plot waveform */
	    plot[k]->selectWaveform(wk[l], states[i], false);
	  }
	}
      }
    }

    /* update table with new select states */
    table->setRowStates(states);

    setCursor("default");
}

void SelfScan::selectRow(void)
{
    int num_selected;
    vector<int> rows;
    gvector<Waveform *> wvec;
    bool select;

    setCursor("hourglass");

    /* get selected table rows and all waveforms in All plot */
    num_selected = table->getSelectedRows(rows);
    plot[0]->getWaveforms(wvec);

    /* determine for each waveform in All plot its select state from table */
    for (int i = 0; i < wvec.size(); i++) {
      int j;
      for (j = 0; j < num_selected && rows[j] != i; j++);

      select = (j < num_selected) ? true : false;

      /* update all plots with the new select state from table */
      for (int k = 0; k < num_clusters + 1; ++k) {
	gvector<Waveform *> wk;
	plot[k]->getWaveforms(wk);

	/* find waveform in each plot corresponding to current waveform in All
	   plot	*/
	for (int l = 0; l < wk.size(); ++l) {
	  int cd_list_id = -1, plot_cd_list_id = -1;
	  wvec[i]->ts->getValue("segment_id", &cd_list_id);
	  wk[l]->ts->getValue("segment_id", &plot_cd_list_id);

	  if (cd_list_id == plot_cd_list_id) {
	    /* set new state for cluster plot waveform */
	    plot[k]->selectWaveform(wk[l], select, false);
	  }
	}
      }
    }

    setCursor("default");
}

void SelfScan::align(void)
{
    double selfscan_time = 0.0, screen_selfscan_time = 0.0;
    vector<double> scaled_x0;
    int id = -1, i, j, n;
    gvector<Waveform *> wvec, wv, wpos;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if( !plot[0]->getWaveforms(wv) || !data_source->getWaveforms(wvec) ) {
	return;
    }

    setCursor("hourglass");

    n = 0;
    for(i = 0; i < wv.size(); i++)
    {
	GTimeSeries *selfscan_ts = wv[i]->ts;
	if( !selfscan_ts->getValue("segment_id", &id) ||
		!selfscan_ts->getValue("selfscan_time", &selfscan_time)) return;

	for(j = 0; j < wvec.size() && n < wv.size(); j++) {
	    if(wvec[j]->getId() == id) {
		if(n == 0) {
		  screen_selfscan_time = wvec[j]->scaled_x0 + selfscan_time
		    - wvec[j]->tbeg();
		}
		else {
		  scaled_x0.push_back(screen_selfscan_time -
				(selfscan_time - wvec[j]->tbeg()));
		  wpos.push_back(wvec[j]);
		}
		n++;
	    }
	}
    }
    if(n > 1) {
	wp->positionX(wpos, scaled_x0);
    }

    setCursor("default");
}

char * SelfScan::getClusterTitle(int clusterid)
{
  char *title = new char[100];

  snprintf(title, 100, cluster_title_format, clusterid);

  return title; 
}

float * SelfScan::getData(GTimeSeries *ts, int *npts)
{
	int i, j, n;
	double tdel = ts->segment(0)->tdel();
	float *r;

	n = 0;
	for(j = 0; j < ts->size(); j++) {
	    if(j  > 0) {
		double gap = ts->segment(j)->tbeg() - ts->segment(j-1)->tend();
		int ngap = (int)(gap/tdel + .5) - 1;
		if(ngap > 0) {
		    n += ngap;
		}
	    }
	    n += ts->segment(j)->length();
	}
	r = (float *)mallocWarn(n*sizeof(float));
	n = 0;
	for(j = 0; j < ts->size(); j++) {
	    if(j  > 0) {
		double gap = ts->segment(j)->tbeg() - ts->segment(j-1)->tend();
		int ngap = (int)(gap/tdel + .5) - 1;
		for(i = 0; i < ngap; i++) {
		    r[n++] = 0.;
		}
	    }
	    for(i = 0; i < ts->segment(j)->length(); i++) {
		r[n++] = ts->segment(j)->data[i];
	    }
	}
	*npts = n;
	return r;
}

