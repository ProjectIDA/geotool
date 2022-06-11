/** \file GCluster.cpp
 *  \brief Defines class GCluster.
 *  \author Vera Miljanovic
 */

#include "config.h"
#include <iostream>
#include <math.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

#include "GCluster.h"
#include "GClusterParams.h"
#include "GClusterSummary.h"
#include "motif++/MotifClasses.h"
#include "libgx++.h"
#include "gobject++/GDataPoint.h"
#include "gobject++/GTimeSeries.h"

extern "C" {
#include "libgmath.h"
#include "libstring.h"
#include "tapers.h"
#include "cluster.h"
}

using namespace libgcluster;

GCluster::GCluster(const char *name, Component *parent,
		 DataSource *ds) : Frame(name, parent, true), DataReceiver(ds)
{
    num_mccc = 0;
    mccc = NULL;
    num_clusters = 0;
    clusterids = NULL;

    /* first (only) argument is clusterid */
    cluster_title_format = strdup("Cluster %d");

    createInterface();
}

void GCluster::createInterface()
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
    summary_button = new Button("Cluster Summary...", view_menu, this);

    option_menu = new Menu("Option", menu_bar);
    parameters_button = new Button("Parameters...", option_menu, this);
    align_button = new Button("Align Waveforms", option_menu, this);
    copy_button = new Button("Copy to Main Window", option_menu, this);
    vandecar_crosson_cluster_s_button =
      new Button("Vandecar Crosson Single-Linkage",
		 option_menu, this);
    shearer_cluster_s_button =
      new Button("Shearer Single-Linkage", option_menu, this);
    vandecar_crosson_cluster_m_button =
      new Button("Vandecar Crosson Maximum-Linkage",
		 option_menu, this);
    shearer_cluster_m_button =
      new Button("Shearer Maximum-Linkage", option_menu, this);

    help_menu = new Menu("Help", menu_bar);
    menu_bar->setHelpMenu(help_menu);
    help_button = new Button("Cluster Help",
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

    param_window = new GClusterParams("Cluster Parameters", this, this);
    summary_window = new GClusterSummary("Cluster Summary", this, this);
    print_window = NULL;

    setButtonsSensitive();

    addPlugins("GCluster", data_source, NULL);

    if(!tool_bar->loadDefaults()) {  // load toolbar after plugins.
	tool_bar->add(close_button, "Close");
	//tool_bar->add(parameters_button, "Parameters...");
	tool_bar->add(vandecar_crosson_cluster_s_button,
		      "Vandecar Crosson Single-Linkage");
	tool_bar->add(shearer_cluster_s_button,
		      "Shearer Single-Linkage");
	tool_bar->add(vandecar_crosson_cluster_m_button,
		      "Vandecar Crosson Maximum-Linkage");
	tool_bar->add(shearer_cluster_m_button,
		      "Shearer Maximum-Linkage");
    }
}

GCluster::~GCluster(void)
{
  clear();
}

void GCluster::actionPerformed(ActionEvent *a)
{
    const char *cmd = a->getActionCommand();
    const char *reason = a->getReason();

    if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Attributes...")) {
	table->showAttributes(true);
    }
    else if(!strcmp(cmd, "Cluster Summary...")) {
      summary_window->setVisible(true);
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
    else if(!strcmp(cmd, "Vandecar Crosson Single-Linkage")) {
	clear();
	compute(VANDECAR_CROSSON, 's');
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Shearer Single-Linkage")) {
	clear();
	compute(SHEARER, 's');
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Vandecar Crosson Maximum-Linkage")) {
	clear();
	compute(VANDECAR_CROSSON, 'm');
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Shearer Maximum-Linkage")) {
	clear();
	compute(SHEARER, 'm');
	setButtonsSensitive();
    }
    else if(!strcmp(cmd, "Align Waveforms")) {
	align();
    }
    else if(!strcmp(cmd, "Copy to Main Window")) {
	copyWaveform();
    }
    else if(!strcmp(cmd, "Cluster Help")) {
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

ParseCmd GCluster::parseCmd(const string &cmd, string &msg)
{
    string c;
    ostringstream os;

    if(parseString(cmd, "all", c) && num_clusters > 0) {
        return plot[0]->parseCmd(c, msg);
    }
    for(int i = 0; i < num_clusters; i++) {
	os.str("");
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
    else if(parseCompare(cmd, "Vandecar_Crosson_Single_Linkage")) {
	vandecar_crosson_cluster_s_button->activate();
    }
    else if(parseCompare(cmd, "Shearer_Single_Linkage")) {
	shearer_cluster_s_button->activate();
    }
    else if(parseCompare(cmd, "Vandecar_Crosson_Maximum_Linkage")) {
	vandecar_crosson_cluster_m_button->activate();
    }
    else if(parseCompare(cmd, "Shearer Maximum_Linkage")) {
	shearer_cluster_m_button->activate();
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

ParseVar GCluster::parseVar(const string &name, string &value)
{
    string c;
    ostringstream os;

    if(parseString(name, "table", c)) {
        return table->parseVar(c, value);
    }
    else if(parseString(name, "all", c) && num_clusters > 0) {
        return plot[0]->parseVar(c, value);
    }

    for(int i = 0; i < num_clusters; i++) {
	os.str("");
	os << "cluster" << i;
	if(parseString(name, os.str(), c)) {
	    return plot[i+1]->parseVar(c, value);
	}
    }
    return Frame::parseVar(name, value);
}

void GCluster::parseHelp(const char *prefix)
{
    printf("%sclear\n", prefix);
    printf("%scluster\n", prefix);
    printf("%sdelete\n", prefix);
    printf("%scopy_to_main_window\n", prefix);
    printf("%svandecar_crosson_cluster_s\n", prefix);
    printf("%sshearer_cluster_s\n", prefix);
    printf("%svandecar_crosson_cluster_m\n", prefix);
    printf("%sshearer_cluster_m\n", prefix);
    printf("%salign waveforms\n", prefix);
    printf("%sprint.help\n", prefix);
    Table::parseHelp(prefix);
}

bool GCluster::getParam(struct GClusterParam_s *cp)
{
    if(!param_window->getInt("number of clusters", &cp->nclusters)) {
      showWarning("Invalid number of clusters");
      return false;
    }
    if(!param_window->getDouble("minimum cross-correlation threshold",
				&cp->min_xcorr_th)) {
      showWarning("Invalid minimum cross-correlation value");
      return false;
    }

    /* Check sanity on ncluster value */
    if (cp->nclusters <= 0) {
      showWarning("Number of clusters must be larger than zero.");
      return false;
    }

    /* Check sanity on min_xcorr_th value */
    if (cp->min_xcorr_th < (double)0.0 || cp->min_xcorr_th > (double)1.0) {
      showWarning("Minimum cross-correlation threshold must be between (inclusive) 0 and 1.");
      return false;
    }



    return true;
}

void GCluster::print(void)
{
    if(print_window == NULL) {
	print_window = new PrintDialog("Print Cluster", this, this);
    }
    print_window->setVisible(true);
}

void GCluster::print(FILE *fp, PrintParam *p)
{
    p->top_title = NULL;
    p->x_axis_label = (char *)"time (seconds)";
    AxesParm *a=NULL;

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

    if(a != NULL) Free(a);
}

void GCluster::compute(const Method mccc_method, const char cluster_method)
{
    double lag;
    GClusterParam cp = GCLUSTER_PARAM_NULL;
    int num_waveforms = 0, select_mode = 0;
    GTimeSeries **ts_list, **mccc_ts_list, *ts, *mccc_ts;
    gvector<Waveform *> wvec;
    double ** ccmatrix = NULL, * ccmatrix_buffer = NULL;
    bool ** ccmatrix_mask = NULL, * ccmatrix_mask_buffer = NULL;
    double ** distmatrix = NULL, * distmatrix_buffer = NULL;
    Node * tree = NULL;
    int n;
    Arg args[30];

    lag = 0.0;

    if (data_source == NULL) {
      showWarning("No data.");
      return;
    }

    if( !getParam(&cp) ) return;

    data_source->getSelectedWaveforms("a", wvec);
    if (wvec.size() < 3) {
      data_source->getSelectedWaveforms(wvec);
      select_mode = 2;
    }
    else {
      select_mode = 1;
    }
    if (wvec.size() < 3) {
      showWarning("Less than 3 waveforms selected.");
      return;
    }

    if (wvec.size() < cp.nclusters) {
      showWarning("Select at least %d waveforms for %d clusters.",
		  cp.nclusters, cp.nclusters);
      return;
    }
    num_waveforms = wvec.size();

    if(!mccc) mccc = (MCCCStruct *)mallocWarn(sizeof(MCCCStruct));

    ccmatrix_buffer = new double[num_waveforms * num_waveforms];
    ccmatrix = new double *[num_waveforms];
    for (int j = 0; j < num_waveforms; ++j) {
      ccmatrix[j] = ccmatrix_buffer + j * num_waveforms;
    }
    ccmatrix_mask_buffer = new bool[num_waveforms * num_waveforms];
    ccmatrix_mask = new bool *[num_waveforms];
    for (int j = 0; j < num_waveforms; ++j) {
      ccmatrix_mask[j] = ccmatrix_mask_buffer + j * num_waveforms;
    }
    mccc = (MCCCStruct *)reallocWarn(mccc, (num_mccc+num_waveforms)*sizeof(MCCCStruct));
    ts_list = new GTimeSeries *[num_waveforms];

    setCursor("hourglass");

    Waveform::sortByY0(wvec);

    for(int i = 0; i < num_waveforms; i++) {
      if (select_mode == 1) {
	double tbeg  = wvec[i]->dw[0].d1->time();
	double tend  = wvec[i]->dw[0].d2->time();
	ts = wvec[i]->ts->subseries(tbeg, tend);
      }
      else if (select_mode == 2) {
	ts = wvec[i]->ts;
      }

      ts_list[i] = ts;
    }

    if (!(mccc_ts_list = multiChannelCCTS(ts_list, num_waveforms,
					  mccc_method, lag, ccmatrix,
					  ccmatrix_mask))) {
      showWarning("Failed to MCCC. See terminal for details.");
    }
    else {
      /* Fill in distmatrix */  
      distmatrix_buffer = new double[num_waveforms * num_waveforms];
      distmatrix = new double *[num_waveforms];
 
      for (int i = 0; i < num_waveforms; i++) {
	distmatrix[i] = distmatrix_buffer + i * num_waveforms; 
	for (int j = 0; j < num_waveforms; j++) {
	  distmatrix[i][j] = (ccmatrix[i][j] - 1) * -1.;
	}
      }

      tree = treecluster(num_waveforms,
			 num_waveforms,
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
	clusterids = new int[num_waveforms];

	cuttree(num_waveforms, tree, cp.nclusters, clusterids);

	//for (int i = 0; i < num_waveforms; ++i) {
	//  printf("DEBUG: waveform %d belongs to cluster %d\n",
	//	 i, clusterids[i]);
	//}

	clear_clusters();

	plot = new WaveformView *[cp.nclusters + 1];

	if (plot == NULL) {
	  showWarning("Failed to allocate memory.");
	}
	else {
	  num_clusters = cp.nclusters;

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

	  /* Fill out summary table */
	  GClusterParam cp = GCLUSTER_PARAM_NULL;
	  if (getParam(&cp)) {
	    int *matched = new int[num_clusters];
	    int *unmatched = new int[num_clusters];

	    /* Initialize matched/unmatched counters */
	    for (int i = 0; i < num_clusters; i++) {
	      matched[i] = 0;
	      unmatched[i] = 0;
	    }

	    /* Count matched and unmatched waveforms */
	    for (int i = 0; i < num_waveforms; i++) {
	      double max_coef = 0.;
	      mccc_ts_list[i]->getValue("max_coef", &max_coef);

	      if (max_coef >= cp.min_xcorr_th) {
		matched[clusterids[i]]++;
	      }
	      else {
		unmatched[clusterids[i]]++;
	      }
	    }

	    /* Fill out table */
	    summary_window->setSummary(num_clusters, matched, unmatched);

	    delete [] matched;
	    delete [] unmatched;
	  }

	  for (int i = 0; i < num_waveforms; i++) {
	    double max_coef = 0.;
	    GClusterParam cp = GCLUSTER_PARAM_NULL;

	    if (select_mode == 1) {
	      ts = ts_list[i];
	      ts->deleteObject();
	    }

	    mccc_ts = mccc_ts_list[i];

	    mccc_ts->putValue("source_id", wvec[i]->getId());
	    mccc_ts->putValue("cluster_id", clusterids[i]);
	    
	    /* if max_coef is >= minimum cross-correlation threshold,
	     * then display waveform
	     */
	    mccc_ts->getValue("max_coef", &max_coef);
	    if (getParam(&cp) && max_coef >= cp.min_xcorr_th) {
	      clusterAdd(wvec[i], mccc_ts, clusterids[i]);

	      double d = mccc_ts->tend() - mccc_ts->tbeg();
	      if (duration_all < d) {
		duration_all = d;
	      }
	      if (duration_cluster[clusterids[i]] < d) {
		duration_cluster[clusterids[i]] = d;
	      }
	    }
	  }

	  delete [] duration_cluster;

	  /* adjust table columns */
	  table->adjustColumns();

	}

	delete [] clusterids;
      }

      free(tree);
      delete [] distmatrix_buffer;
      delete [] distmatrix;
    }

    tab->setOnTop("All");

    setCursor("default");

    delete [] ccmatrix_mask_buffer;
    delete [] ccmatrix_mask;
    delete [] ccmatrix_buffer;
    delete [] ccmatrix;
    delete [] ts_list;
    delete [] mccc_ts_list;
}

void GCluster::clusterAdd(Waveform *w, GTimeSeries *mccc_ts, int clusterid)
{
    char clusterid_string[20], maxMCCC[20], time[50], absCorrLag[50],
      relCorrLag[50], chan[20];
    GDataPoint *dmin, *dmax, *dp;
    
    mccc_ts->setSta(w->sta());
    snprintf(chan, sizeof(chan),"%sC", w->chan());
    mccc_ts->setChan(chan);
    mccc_ts->setNet(w->net());
    plot[0]->addWaveform(mccc_ts, w->fg);
    plot[clusterid + 1]->addWaveform(mccc_ts, w->fg);

    dmin = mccc_ts->minPoint();
    dmax = mccc_ts->maxPoint();

    dp = (fabs(dmin->value()) > fabs(dmax->value())) ? dmin : dmax;

    strcpy(mccc[num_mccc].sta, w->sta());
    strcpy(mccc[num_mccc].chan, w->chan());
    mccc_ts->getValue("max_coef", &mccc[num_mccc].max_coef);
    mccc[num_mccc].time = dp->time();
    mccc[num_mccc].abs_corr_lag = dp->time();
    mccc[num_mccc].rel_corr_lag = dp->time() - mccc[0].time;

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
	    row[i] = mccc[num_mccc].sta;
	}
	else if(!strcmp(a.name, "chan")) {
	    row[i] = mccc[num_mccc].chan;
	}
	else if(!strcmp(a.name, "max coef")) {
	    snprintf(maxMCCC, sizeof(maxMCCC), a.format, mccc[num_mccc].max_coef);
	    row[i] = maxMCCC;
	}
	else if(!strcmp(a.name, "time")) {
	    if(!strcmp(a.format, "%t")) {
		row[i] = timeEpochToString(mccc[num_mccc].time, time,
					   sizeof(time),
					   YMONDHMS);
	    }
	    else {
		snprintf(time, sizeof(time), a.format, mccc[num_mccc].time);
	    }
	}
	else if (!strcmp(a.name, "abs corr lag")) {
	    strcpy(absCorrLag, "-999.9");
	    if(table->numRows() > 0) {
		snprintf(absCorrLag, sizeof(absCorrLag), a.format,
			 mccc[num_mccc].abs_corr_lag);
	    }
	    row[i] = absCorrLag;
	}
	else if (!strcmp(a.name, "rel corr lag")) {
	    strcpy(relCorrLag, "-999.9");
	    if(table->numRows() > 0) {
		snprintf(relCorrLag, sizeof(relCorrLag), a.format,
			 mccc[num_mccc].rel_corr_lag);
	    }
	    row[i] = relCorrLag;
	}

    }

    mccc_ts->putValue("mccc_max_coef", mccc[num_mccc].max_coef);
    mccc_ts->putValue("mccc_time", mccc[num_mccc].time);
    mccc_ts->putValue("mccc_abs_corr_lag", mccc[num_mccc].abs_corr_lag);
    mccc_ts->putValue("mccc_abs_corr_lag", mccc[num_mccc].rel_corr_lag);
    dmin->deleteObject();
    dmax->deleteObject();

    table->addRow(row, false);

    delete [] row;

    num_mccc++;

}

void GCluster::list(void)
{
    char maxMCCC[20], time[50], timeLag[50];
    double t0 = mccc[0].time;

    table->removeAllRows();
    int num_columns = table->numColumns();
    const char **row = new const char *[num_columns];

    for(int j = 0; j < num_mccc; j++) {
	for(int i = 0; i < num_columns; i++)
	{
	    TAttribute a = table->getAttribute(i);
	    if(!strcmp(a.name, "sta")) {
		row[i] = mccc[j].sta;
	    }
	    else if(!strcmp(a.name, "chan")) {
		row[i] = mccc[j].chan;
	    }
	    else if(!strcmp(a.name, "max coef")) {
		snprintf(maxMCCC, sizeof(maxMCCC), a.format, mccc[j].max_coef);
		row[i] = maxMCCC;
	    }
	    else if(!strcmp(a.name, "time")) {
		if(!strcmp(a.format, "%t")) {
		    row[i] = timeEpochToString(mccc[j].time, time, sizeof(time),
					YMONDHMS);
		}
		else {
		    snprintf(time, sizeof(time), a.format, mccc[j].time);
		}
	    }
	    else {
		snprintf(timeLag, sizeof(timeLag), a.format,
				mccc[j].time - t0);
		row[i] = timeLag;
	    }
	}
	table->addRow(row, false);
    }
    delete row;
    table->adjustColumns();
}

void GCluster::setButtonsSensitive(void)
{
  bool sensitive = (table->numRows() > 0) ? true : false;
  align_button->setSensitive(sensitive);
  clear_button->setSensitive(sensitive);
  
  sensitive = table->rowSelected();
  copy_button->setSensitive(sensitive);
  delete_button->setSensitive(sensitive);
}

void GCluster::copyWaveform(void)
{
    GTimeSeries **mccc_ts = NULL;
    gvector<Waveform *> wvec;
    Pixel *pixels = NULL;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    /* just using plot[0] (All) works as it is always updated with what is
     * selected elsewhere as well
     */
    if(plot[0]->getSelectedWaveforms(wvec) <= 0) {
	showWarning("No cluster waveforms selected.");
	return;
    }

    mccc_ts = new GTimeSeries * [wvec.size()];
    pixels = new Pixel[wvec.size()];

    for(int i = 0; i < wvec.size(); i++) {
	mccc_ts[i] = new GTimeSeries(wvec[i]->ts);
	pixels[i] = wvec[i]->fg;
    }
    wp->addWaveforms(wvec.size(), mccc_ts, NULL, pixels);

    delete [] pixels;
    delete [] mccc_ts;
}

void GCluster::clear_clusters(void)
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

void GCluster::clear(void)
{
  setCursor("hourglass");

  clear_clusters();

  num_mccc = 0;
  Free(mccc); mccc = NULL;

  setButtonsSensitive();

  setCursor("default");
}

void GCluster::remove(void)
{
    int n;
    vector<int> rows;
    gvector<Waveform *> w_sel, ws, wdel;

    /* just using plot[0] (All) works as it is always updated with what is
     * selected elsewhere as well
     */
    if(plot[0]->getSelectedWaveforms(w_sel) <= 0) {
	showWarning("No cluster waveforms selected.");
	return;
    }

    setCursor("hourglass");

    /* delete the selected waveforms from the table */
    plot[0]->getWaveforms(ws);
    n = 0;
    for (int i = 0; i < ws.size(); i++) {
      int j;
      for (j = 0; j < w_sel.size() && ws[i] != w_sel[j]; j++);

      if(j < w_sel.size()) {
	rows.push_back(i);
	n++;
      }
    }
    table->removeRows(rows);

    /* delete selected waveforms from all plots */

    for (int i = num_clusters; i >= 0; --i) {  /* touch plot[0] last */
      plot[i]->getWaveforms(ws);

      wdel.clear();
      for (int j = 0; j < ws.size(); ++j) {
	int ws_id = -1, wvec_id, k;
	ws[j]->ts->getValue("source_id", &ws_id);
	for(k = 0; k < w_sel.size()
	      && w_sel[k]->ts->getValue("source_id", &wvec_id)
	      && ws_id != wvec_id; k++);

	if (k < w_sel.size()) {
	  wdel.push_back(ws[j]);
	}
      }

      plot[i]->deleteWaveforms(wdel);
    }

    /* remove waveforms from mccc array */
    for(int i = num_mccc-1; i >= 0; i--) {
      int j;
      for(j = 0; j < n && rows[j] != i; j++);

      if(j < n) {
	// delete row i;
	for(j = i; j < num_mccc-1; j++) {
	  mccc[j] = mccc[j+1];
	}
	num_mccc--;
      }
    }

    setCursor("default");
}

void GCluster::selectWaveform(void)
{
    vector<bool> states;
    int num_selected = 0;
    gvector<Waveform *> wvec, ws;

    /* Maximum number of waveforms/states */
    if(plot[0]->getWaveforms(ws) <= 0) return;

    setCursor("hourglass");

    /* work on only current visible waveform plot (tab) */
    char *ontop = tab->labelOnTop();

    if (strcmp(ontop, "All") == 0) {
      num_selected = plot[0]->getSelectedWaveforms(wvec);
    }
    else {
      for (int i = 0; i < num_clusters; ++i) {
	char *title = getClusterTitle(i);
	if (strcmp(ontop, title) == 0) {
	  num_selected = plot[i + 1]->getSelectedWaveforms(wvec);
	  delete [] title;
	  break;
	}
	delete [] title;
      }
    }

    /* prepare new row state array for the table (and plots) update */
    for(int i = 0; i < ws.size(); i++) {
      int ws_id = -1, wvec_id, j;
      /* cannot compare ws w/ wvec as Waveform object is a duplicate
	 for(j = 0; j < num_selected && ws[i] != wvec[j]; j++);
      */
      ws[i]->ts->getValue("source_id", &ws_id);
      for(j = 0; j < num_selected
	    && wvec[j]->ts->getValue("source_id", &wvec_id)
	    && ws_id != wvec_id; j++);

      states.push_back((j < num_selected) ? true : false);

      /* update all plots with the new select state  */
      for (int k = 0; k < num_clusters + 1; ++k) {
	gvector<Waveform *> plot_wvec;
	int num_wf_plot = plot[k]->getWaveforms(plot_wvec);

	/* find waveform in each plot corresponding to current waveform in All
	   plot */
	for (int l = 0; l < num_wf_plot; ++l) {
	  int plot_wvec_id = -1;
	  plot_wvec[l]->ts->getValue("source_id", &plot_wvec_id);

	  if (ws_id == plot_wvec_id) {
	    /* set new state for plot waveform */
	    plot[k]->selectWaveform(plot_wvec[l], states[i], false);
	  }
	}
      }
    }

    /* update table with new select states */
    table->setRowStates(states);

    setCursor("default");
}

void GCluster::selectRow(void)
{
    int num_waveforms, num_selected;
    vector<int> rows;
    gvector<Waveform *> wvec;
    bool select;

    setCursor("hourglass");

    /* get selected table rows and all waveforms in All plot */
    num_selected = table->getSelectedRows(rows);
    num_waveforms = plot[0]->getWaveforms(wvec);

    /* determine for each waveform in All plot its select state from table */
    for (int i = 0; i < num_waveforms; i++) {
      int j;
      for (j = 0; j < num_selected && rows[j] != i; j++);

      select = (j < num_selected) ? true : false;

      /* update all plots with the new select state from table */
      for (int k = 0; k < num_clusters + 1; ++k) {
	gvector<Waveform *> plot_wvec;
	int num_wf_plot = plot[k]->getWaveforms(plot_wvec);

	/* find waveform in each plot corresponding to current waveform in All
	   plot	*/
	for (int l = 0; l < num_wf_plot; ++l) {
	  int wvec_id = -1, plot_wvec_id = -1;
	  wvec[i]->ts->getValue("source_id", &wvec_id);
	  plot_wvec[l]->ts->getValue("source_id", &plot_wvec_id);

	  if (wvec_id == plot_wvec_id) {
	    /* set new state for cluster plot waveform */
	    plot[k]->selectWaveform(plot_wvec[l], select, false);
	  }
	}
      }
    }

    setCursor("default");
}

void GCluster::align(void)
{
    double mccc_time=0., screen_mccc_time=0.;
    int id = -1, i, j, num, num_waveforms, n;
    gvector<Waveform *> wvec, ws, c;
    vector<double> scaled_x0;

    if( !(wp = data_source->getWaveformPlotInstance()) ) return;

    if(!(num = plot[0]->getWaveforms(ws))) return;

    if(!(num_waveforms = data_source->getWaveforms(wvec))) {
	return;
    }

    setCursor("hourglass");

    n = 0;
    for(i = 0; i < num; i++)
    {
	GTimeSeries *mccc_ts = ws[i]->ts;
	if( !mccc_ts->getValue("source_id", &id) ||
		!mccc_ts->getValue("mccc_time", &mccc_time)) return;

	for(j = 0; j < num_waveforms && n < num; j++) {
	    if(wvec[j]->getId() == id) {
		if(n == 0) {
		  screen_mccc_time = wvec[j]->scaled_x0 + mccc_time
		    - wvec[j]->tbeg();
		}
		else {
		  scaled_x0.push_back(screen_mccc_time -
				(mccc_time - wvec[j]->tbeg()));
		  c.push_back(wvec[j]);
		}
		n++;
	    }
	}
    }
    if(n > 1) {
	wp->positionX(c, scaled_x0);
    }

    setCursor("default");
}

char * GCluster::getClusterTitle(int clusterid)
{
  char *title = new char[100];

  snprintf(title, 100, cluster_title_format, clusterid);

  return title; 
}

float * GCluster::getData(GTimeSeries *ts, int *npts)
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
