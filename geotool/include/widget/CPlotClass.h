#ifndef _CPLOT_CLASS_H_
#define _CPLOT_CLASS_H_

#include "widget/Parser.h"
#include "widget/AxesClass.h"
#include "Waveform.h"
#include "widget/TableListener.h"
#include "widget/CPlot.h"

class GTimeSeries;
class MagnifyWindow;
class InfoArea;

/** This class is the interface to the CPlot Widget.
 *
 *  @ingroup libwgets
 */
class CPlotClass : public AxesClass, public Parser
{
    public:

    CPlotClass(const string &name, Component *parent, Arg *args=NULL, int n=0);
    CPlotClass(const string &name, Component *parent, InfoArea *infoarea,
			Arg *args, int n);
    ~CPlotClass(void);

    void actionPerformed(ActionEvent *action_event);

    void destroy(void);

    void initWidget(InfoArea *, Arg *args, int n, bool reset_y_limits=true);

    /**
     *  Add a curve. The curve will be displayed immediately. The limits
     *  of the axes will adjust according to the parameter adjust_limits.
     *  Possible values for adjust_limits are:
     *  - CPLOT_HOLD_LIMITS  Do not change the current limits of the axes.
     *  - CPLOT_ADJUST_X_LIMITS Adjust only the x axis limits, if necessary.
     *  - CPLOT_ADJUST_Y_LIMITS Adjust only the y axis limits, if necessary.
     *  - CPLOT_ADJUST_LIMITS Adjust the x axis and y-axis limits, if necessary.
     *  - CPLOT_ADJUST_Y_GROW Allow the y axis limits to grow, but not shrink.
     *  @param[in] npts The number of points on the curve.
     *  @param[in] x The x or horizontal coordinates of the curve.
     *  @param[in] y The y or vertical coordinates of the curve.
     *  @param[in] type The type can be <TT>CPLOT_CURVE</TT> for a regular
     *  	curve, or <TT>CPLOT_TTCURVE</TT> for a travel time curve.
     *  @param[in] lab A label that will be printed next to the curve.
     *  @param[in] mouse_label A label that will be printed in a separate
     *	text window when the mouse cursor is moved near to the curve.
     *  @param[in] on The initial visiblity of the curve. If on is true,
     *	the curve is drawn immediately, otherwise the curve is not
     *	drawn until a later function call changes this parameter.
     *  @param[in] adjust_limits Controls how the axes limits are adjusted
     *		to display this curve.
     *  @param[in] pixel The curve will be drawn using this color.
     */
    bool addCurve(int npts, double *x, float *y, int type, const string &lab,
	const string &mouse_label, bool on, short adjust_limits, Pixel pixel);
    bool addCurve(int npts, double *x, float *y, Pixel pixel) {
	return addCurve(npts, x, y, CPLOT_CURVE, "", "", true,
		CPLOT_ADJUST_LIMITS, pixel);
    }
    bool addCurve(int npts, double *x, float *y, const string mouse_label,
		bool on, Pixel pixel) {
	return addCurve(npts, x, y, CPLOT_CURVE, "", mouse_label, on,
		CPLOT_ADJUST_LIMITS, pixel);
    }
    /** Get all curves in a C++ vector.
     *  @param[out] curves an allocated vector of CPlotCurve structures.
     *  @returns the number of curves.
     */
    int getCurves(vector<CPlotCurve> &curves);

    /** Get all curves in a C array.
     *  @param[out] curves an allocated array of CPlotCurve structures.
     *  @returns the number of curves.
     */
    int getCurves(CPlotCurve **curves) {
      return CPlotGetCurves(cw, curves);
    }

    /** Change a curve.
     *  @param[in] curve_index the index of the curve.
     *  @param[in] npts the number of values.
     *  @param[in] x the new x values.
     *  @param[in] y the new y values.
     *  @returns true for success.
     */
    bool changeCurve(int curve_index, int npts, double *x, float *y);
    /** Set the visible state of a curve.
     *  @param[in] curve_index the index of the curve.
     *  @param[in] visible if true, the curve is drawn.
     *  @param[in] redisplay if true, the curve is drawn immediately.
     */
    void setCurveVisible(int curve_index, bool visible, bool redisplay);
    /** Get the number of curves.
     *  @returns the number of curves.
     */
    int numCurves(void);
    /**
     *  Add a GTimeSeries to a CPlotWidget. The CPlotInputStruct holds the
     *  parameters that control how the waveform will be displayed in the
     *  CPlotWidget window.
     *  @param[in] ts a GTimeSeries object.
     *  @param[in] input a structure with waveform information.
     *    - input->display_t0 The time at which to position the first
     *		sample of the TimeSeries.
     *    - input->tag The tag or label to be displayed next to the
     *		TimeSeries.
     *    - input->sta The station name for the TimeSeries.
     *    - input->chan The channel name for the TimeSeries.
     *    - input->net The network name for the TimeSeries.
     *    - input->lat The station latitude in degrees. Can be set to -999.
     *    - input->lon The station longitude in degrees. Can be set to -999.
     *    - input->origin An optional origin to be associated with the
     *		TimeSeries. Can be NULL.
     *    - input->pixel The color used to draw the TimeSeries.
     *    - input->on The initial visibility of the TimeSeries. If on is
     *		true, the TimeSeries is drawn immediately, otherwise it
     *		is not drawn until a later function call changes this
     *                      parameter.
     *  @returns A Waveform object for the TimeSeries, or NULL if the
     *		TimeSeries has no samples or one of the
     *              CPlotInputStruct parameters is invalid.
     */
    virtual Waveform *addTimeSeries(GTimeSeries *ts, CPlotInputStruct *input);
    /** Find the waveform that an arrival is associated with.
     *  @param[in] arrival a CssArrivalClass that has been added.
     *  @returns The Waveform pointer corresponding to the waveform that
     *		the arrival is associated with. Returns NULL if the arrival is
     *		not found.
     */
    Waveform *getArrivalData(CssArrivalClass *arrival);
    /** Get all wftags contained in this object.
     *  @param[out] wftags An allocated array of CssWftag objects.
     *  @returns the number of CssWftag objects.
     */
    int getTable(cvector<CssAmpdescriptClass> &v){return getTable(cssAmpdescript,v);}
    int getTable(cvector<CssAmplitudeClass> &v) { return getTable(cssAmplitude, v);}
    int getTable(cvector<CssArrivalClass> &v)   { return getTable(cssArrival, v);}
    int getTable(cvector<CssAssocClass> &v)     { return getTable(cssAssoc, v);}
    int getTable(cvector<CssFilterClass> &v)    { return getTable(cssFilter, v);}
    int getTable(cvector<CssHydroFeaturesClass> &v) {
	return getTable(cssHydroFeatures, v);}
    int getTable(cvector<CssInfraFeaturesClass> &v) {
	return getTable(cssInfraFeatures, v);}
    int getTable(cvector<CssNetmagClass> &v)   { return getTable(cssNetmag, v); }
    int getTable(cvector<CssOriginClass> &v)   { return getTable(cssOrigin, v); }
    int getTable(cvector<CssOrigerrClass> &v)  { return getTable(cssOrigerr, v); }
    int getTable(cvector<CssParrivalClass> &v) { return getTable(cssParrival, v); }
    int getTable(cvector<CssStamagClass> &v)   { return getTable(cssStamag, v);}
    int getTable(cvector<CssStassocClass> &v)  { return getTable(cssStassoc, v); }
    int getTable(cvector<CssWftagClass> &v)    { return getTable(cssWftag, v); }
    int getTable(cvector<CssWfdiscClass> &v)   { return getTable(cssWfdisc, v); }
    int getTable(cvector<CssAffiliationClass> &v) { v.clear(); return 0; }
    int getTable(cvector<CssSiteClass> &v) { v.clear(); return 0; }
    int getTable(cvector<CssSitechanClass> &v) { v.clear(); return 0; }
    int getTable(cvector<CssXtagClass> &v) { v.clear(); return 0; }

    int getTable(const string &table_name, gvector<CssTableClass *> &v);

    gvector<CssTableClass *> * getTableRef(const string &table_name);

    int getSelectedTable(cvector<CssArrivalClass> &v);
    int getSelectedTable(cvector<CssOriginClass> &v);

    int getSelectedTable(cvector<CssAmpdescriptClass> &v) { return 0; }
    int getSelectedTable(cvector<CssAmplitudeClass> &v) { return 0; }
    int getSelectedTable(cvector<CssAssocClass> &v) { return 0; }
    int getSelectedTable(cvector<CssFilterClass> &v) { return 0; }
    int getSelectedTable(cvector<CssHydroFeaturesClass> &v) { return 0; }
    int getSelectedTable(cvector<CssInfraFeaturesClass> &v) { return 0; }
    int getSelectedTable(cvector<CssNetmagClass> &v) { return 0; }
    int getSelectedTable(cvector<CssOrigerrClass> &v) { return 0; }
    int getSelectedTable(cvector<CssParrivalClass> &v) { return 0; }
    int getSelectedTable(cvector<CssStamagClass> &v) { return 0; }
    int getSelectedTable(cvector<CssStassocClass> &v) { return 0; }
    int getSelectedTable(cvector<CssWftagClass> &v) { return 0; }
    int getSelectedTable(cvector<CssWfdiscClass> &v) { return 0; }
    int getSelectedTable(cvector<CssSiteClass> &v) { return 0; }
    int getSelectedTable(cvector<CssSitechanClass> &v) { return 0; }
    int getSelectedTable(cvector<CssAffiliationClass> &v) { return 0; }
    int getSelectedTable(cvector<CssXtagClass> &v) { return 0; }

    int getSelectedTable(const string &table_name, gvector<CssTableClass *> &v) {
	if(!table_name.compare(cssArrival)) {
	    return getSelectedTable((cvector<CssArrivalClass> &)v);
	}
	else if(!table_name.compare(cssOrigin)) {
	    return getSelectedTable((cvector<CssOriginClass> &)v);
	}
	return -1;
    }

    cvector<CssArrivalClass> * getArrivalRef(void);
    cvector<CssOriginClass> * getOriginRef(void);
    cvector<CssOrigerrClass> * getOrigerrRef(void);
    cvector<CssAssocClass> * getAssocRef(void);
    cvector<CssStassocClass> * getStassocRef(void);
    cvector<CssStamagClass> * getStamagRef(void);
    cvector<CssNetmagClass> * getNetmagRef(void);
    cvector<CssHydroFeaturesClass> * getHydroFeaturesRef(void);
    cvector<CssInfraFeaturesClass> * getInfraFeaturesRef(void);
    cvector<CssWftagClass> * getWftagRef(void);
    cvector<CssAmplitudeClass> * getAmplitudeRef(void);
    cvector<CssAmpdescriptClass> * getAmpdescriptRef(void);
    cvector<CssParrivalClass> * getParrivalRef(void);
    cvector<CssFilterClass> * getFilterRef(void);

    /** Align all waveforms with their first sample at time 0.
     */
    void alignFirstPt(void);
    /** Set waveform colors. Specify the Pixel color of individual waveforms.
     *  @param[in] num_waveforms The number of waveforms referenced in wvec.
     *  @param[in] wvec An array of Waveform objects.
     *  @param[in] fg An array of num_waveforms Pixels.
     *  @param[in] redisplay If true, the data window is immediately redrawn.
     *              If false, the data is not redrawn until some other CPlot
     *              function causes a waveform redraw.
     */
    void colorWaveforms(gvector<Waveform *> &wvec, Pixel *fg, bool redisplay);
    /** Change the color of selected waveforms. The data window will be
     *  immediately redrawn. The selected waveforms will remain selected, but
     *  their deselected color will be set to the input Pixel color.
     *  @param[in] color The color to be assigned to all currently selected
     *      waveforms.
     */
    void changeColor(Pixel color);
    /** Align all waveforms on a phase-line-cursor.
     *  @param[in] type The type of alignment:
     *  - CPLOT_SELECTED_WAVEFORM align all selected waveforms that contain the
     *                      phase.
     *  - CPLOT_SELECTED_ARRIVAL align all waveforms that have the phase-line
     *              arrival selected.
     *  - CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM align all selected waveforms
     *              that have the phase-line arrival selected.
     *  If the resource XtNalignArrSelected is false, then this routine ignores
     *  the selected state of the waveforms and aligns all waveforms that
     *  contain the arrival (in the case of CPLOT_SELECTED_WAVEFORM) or contain
     *  the selected arrival (in the cases of CPLOT_SELECTED_ARRIVAL and
     *  CPLOT_SELECTED_ARRIVAL_SELECTED_WAVEFORM).
     * @returns the arrival phase name. Do not free it.
     */
    const char *alignOnArrival(int type);
    /** Remove a waveform amplitude measurement box. Only the measurement box
     *  displayed on the specified waveform is removed. Measurement boxes on
     *  other waveforms are not removed.
     *  @param[in] w the Waveform pointer associated with the waveform.
     */
    void removeMeasureBox(Waveform *w);
    /** Remove all amplitude measure boxes.
     */
    void removeAllMeasureBoxes() {
	CPlotRemoveAllMeasureBoxes(cw); }
    /** Set the waveform vertical scaling factor. Set the data height of
     *  selected waveforms. This does not effect the initial data height for
     *  new waveforms, which is set by setDataHeight().
     *  @param[in] data_height the data height in pixels.
     *  @param[in] visible_only if true, set the data height for waveforms
     *		currently visible in the data window, subject to the 'all'
     *  	parameter. If false, set the data height according to the 'all'
     *		parameter.
     *  @param[in] all if true, set the data height for all waveforms. If false,
     *		set the data height for selected waveforms only. (Subject to
     *		the visible_only parameter.)
     */
    void setScale(int data_height, bool visible_only, bool all);
    /** Select/deselect waveforms. Select or deselect all waveforms, or
     *  only visible waveforms or only waveforms with a specific channel type.
     *  Possible values of chan_type are:
     *  - CPLOT_ALL_COMPONENTS select all components.
     *  - CPLOT_Z_COMPONENT select z components only.
     *  - CPLOT_HORIZONTAL_COMPONENTS select horizontal components only.
     *  @param[in] on_off If true, select waveforms. If false, deselect
     *	waveforms.
     *  @param[in] visible_only if true, select only visible waveforms,
     *  subject to chan_type.
     *  @param[in] chan_type the component selection parameter.
     */
    void selectAll(bool on_off, bool visible_only=false,
		int chan_type=CPLOT_ALL_COMPONENTS) {
	CPlotSelectAll(cw, on_off, visible_only,chan_type);
    }
    /** Initialize the magnify window. This function is called each time the
     *  magnify window is popuped up.
     *  @param[in] z a CPlotWidget pointer to the magnify window.
     *  @param[in] redisplay if redisplay is false, the magnify window is not
     *  	updated. Otherwise it is.
     */
    void magnifyWidget(CPlotWidget z, bool redisplay) {
	CPlotMagnifyWidget(cw, z, (Boolean)redisplay);
    }

    void hideMagWindow(void);

    /** Place amplitude measurement boxes on waveforms at the line-cursor
     *  position. Amplitude measurement boxes are drawn on waveforms at the
     *  position of a phase-line-cursor created with addPhaseLine().
     *  @param[in] selected if true, boxes are drawn only on selected waveforms,
     * 		otherwise they are drawn on all waveforms.
     *  @param[in] with_arrival if true, the first arrival of waveforms on which
     * 		a box is drawn is selected.
     *  @returns true if any boxes were drawn.
     */
    bool measureAll(bool selected, bool with_arrival);
    /** Search selected waveforms for a cycle within period limits. Search
     *  waveforms within a double-line cursor to find the first occurrence
     *  of a cycle with a period that is between min_per and max_per.
     *  Draw a measurement box on the first qualifying cycle of each waveform.
     *  @param[in] min_per the minimum period in seconds.
     *  @param[in] max_per the maximum period in seconds.
     *  @param[in] do_largest_amp if true, and no cycle is found with a
     *	    qualifying period, then a measure box is drawn on the cycle with the
     *	    largest amplitude. If False, no measure box is drawn on waveforms
     *	    that do not have a qualifying period within the double-line cursors.
     *  @returns true if any boxes were drawn.
     */
    int search(double min_per, double max_per, bool do_largest_amp);
    /** Get a measurement box on a waveform that is associated with an arrival.
     *  Returns the measurement box currently displayed on a waveform that is
     *  associated with the specified arrival. If more than one waveform that is
     *  associated with the arrival has a measurement box, then the box
     *  information return will be for the last measurement box found.
     *  @param[in] arrival a CssArrivalClass associated with one or more waveforms in
     *  		this CPlotClass.
     *  @param[out] m a CPlotMeasure structure that will be filled.
     *  @returns the number of boxes found on waveforms associated with
     *			the specified arrival.
     */
    int getArrivalMeasureBox(CssArrivalClass *arrival, CPlotMeasure *m);
    /** Get the measurement box information for the specified  waveform. If a
     *  measurement box is displayed on the specified waveform, its dimensions
     *  are returned in the CPlotMeasure structure.
     *  @param[in] w a Waveform pointer for a waveform contained in this
     *  	CPlotClass.
     *  @param[out] m a CPlotMeasure structure that will be filled.
     *  @returns true, if the waveform has a measurement box.
     */
    bool getWaveformMeasureBox(Waveform *w, CPlotMeasure *m);
    /** Draw a measurement box on a waveform using for the specified arrival.
     *  A measurement box with the amplitude and period of the specified
     *  CssArrivalClass structure is drawn on all associated waveforms. This function
     *   causes the XtNmeasureCallback calback functions to be called.
     *  @param[in] arrival an arrival that is associated with on or more
     *		waveforms in this CPlotClass.
     */
    void reviewMeasurements(bool selected, CssArrivalClass *arrival);
    void reviewMeasurement(CssArrivalClass *arrival) {
	reviewMeasurements(false, NULL);
    }
    void reviewMeasurementOnWaveform(CssAmplitudeClass *amplitude, Waveform *w);
    void addMeasurement(CPlotMeasure *m, Waveform *w);
    /** Draw all measurement boxes for all selected arrivals on selected
     *  waveforms. Measurement boxes are drawn using the amplitude and period
     *  of selected arrivals. The box is only drawn if the waveform is also
     *  selected. This function causes the XtNmeasureCallback calback functions
     *  to be called.
     */
    void reviewSelectedMeasurements() { reviewMeasurements(true, NULL); }
    /** Get the Waveform objects for all selected waveforms.
     *  @param[out] wvec an allocated array of Waveform objects.
     *      The members of the Waveform objects should not be altered,
     *	    since they continue to be used by the CPlotClass.
     *  @returns the number of Waveform objects in wvec.
     */
    int getCompGroup(const string &in_sta, double t, double l,
		const string &components, gvector<Waveform *> &wvec);
    int getSelectedComps(const string &components, gvector<Waveform *> &wvec);
    void selectWaveform(Waveform *w, bool select, bool do_callbacks=false);
    void selectAllArrivals(bool select);
    void arrivalSetColor(CssArrivalClass *arrival, Pixel col);
    void displayData(int mode);
    void positionOne(Waveform *w, double scaled_x0, double scaled_y0);
    void positionX2(gvector<Waveform *> &wvec, vector<double> &scaled_x0,
			double x0_min, double x0_max);
    void modify(gvector<Waveform *> &wvec, char **new_tags, bool redisplay);
    void modify(Waveform *w) {
	gvector<Waveform *> v(w);
	modify(v, NULL, true);
    }
    CssOriginClass *getPrimaryOrigin(Waveform *w);

    void selectOrigin(CssOriginClass *origin, bool selected) {
	if(selectTable((CssTableClass *)origin, selected)) {
	    change.select_origin = true;
	    doDataChangeCallbacks();
	}
    }

    void removeTable(CssTableClass *table, bool do_callback=true);
    void clearTable(string &table_name, bool do_callback=true);

    void renameArrival(CssArrivalClass *arrival, const string &phase,
		bool do_callback=true)
    {
	CPlotRenameArrival(cw, arrival, (char *)phase.c_str());
	if(do_callback) {
	    change.arrival = true;
	    doDataChangeCallbacks();
	    int member = arrival->memberIndex("iphase");
	    TableListener::doCallbacks(arrival, this, member);
	}
    }
    bool addArrivalType(CPlotArrivalType *type);
    void arrivalAssociated(CssArrivalClass *arrival, bool associated);
    void renameAssoc(CssAssocClass *assoc, CssArrivalClass *arrival, const string &phase,
		bool do_callback=true)
    {
	CPlotRenameArrival(cw, arrival, (char *)phase.c_str());
	if(do_callback) {
	    change.assoc = true;
	    doDataChangeCallbacks();
	    int member = assoc->memberIndex("phase");
	    TableListener::doCallbacks(assoc, this, member);
	}
    }
    void clearArrivals(bool do_callback=true);
    bool selectArrival(CssArrivalClass *arrival, bool select);
    bool selectArrivalWithCB(CssArrivalClass *arrival, bool select);
    void colorArrival(CssArrivalClass *arrival, Pixel fg);
    bool arrivalState(CssArrivalClass *arrival);
    void moveArrival(CssArrivalClass *arrival, double time,bool do_callback=false);
    void setArrivalModified(CssArrivalClass *a, bool modified);
    void changeTag(Waveform *w, const string &tag, bool redisplay);
    void minMax(Waveform *w, double *min, double *max);
    bool dataVisible(void);
    void zoomOut(void) { CPlotZoomOut(cw); }
    void scrollBottom(Waveform *w);
    CssOriginClass *dataToOrigin(GTimeSeries *ts);
    void getTagDimensions(const string &tag, int *width, int *height) {
	CPlotGetTagDimensions(cw, tag, width, height);
    }
    int getMaxTagWidth(void);
    int getMaxAmpWidth(void);
    void setMaxTagWidth(int width);
    void setYLabelInt(bool set, bool redisplay);
    int getDataHeight(void);
    int getDataSeparation(void);

    virtual void positionX(gvector<Waveform *> &wvec,vector<double> &scaled_x0);
    virtual void positionY(gvector<Waveform *> &wvec,vector<double> &scaled_y0);
    virtual void positionXY(gvector<Waveform *> &wvec,
			vector<double> &scaled_x0, vector<double> &scaled_y0);
    virtual bool setPrimaryOrigin(Waveform *w, CssOriginClass *origin);
    virtual long getWorkingOrid(void);
    virtual void setWorkingOrid(long orid, bool do_callback=true);
    virtual void deleteWaveforms(gvector<Waveform *> &wvec);
    virtual void deleteWaveforms_NoReposition(gvector<Waveform *> &wvec);
    virtual void deleteSelectedWaveforms(void);
    virtual void clear(bool do_callback=true);
    virtual void clearWaveforms(bool do_callback=true);
    virtual void getTimeLimits(double *tmin, double *tmax);
    virtual void setTimeLimits(double tmin, double tmax) {
	CPlotSetTimeLimits(cw, tmin, tmax);
    }
    virtual bool getDataDuration(double *tmin, double *tmax) {
	return CPlotGetDataDuration(cw, tmin, tmax); }
    virtual void setDataHeight(int dataHeight, int dataSeparation);
    virtual void alignOnPhase(const string &phase);
    virtual void setWaveformOrder(gvector<Waveform *> &wvec,
			bool redisplay=true);
    virtual void displayComponents(int display);
    virtual void setDataDisplay(gvector<Waveform *> &wvec, vector<bool> &on,
		bool promote_visible, bool redisplay);

    void timeShift(double time_shift);
    int getComponentDisplayMode(void);
    void setUniformScale(bool set);
    bool getUniformScale(void);
    void displayArrival(const string &sta, const string &chan,int arid,bool on);
    void setArrivalOn(CssArrivalClass *a, bool on);
    void setArrivalsDisplay(cvector<CssArrivalClass> &a, bool *on);
    void scaleUp(bool all);
    void scaleDown(bool all);
    const char *getTag(Waveform *w);
    void setCPlotArrivalKeys(vector<ArrivalKey> &keys);

    ParseCmd parseCmd(const string &cmd, string &msg);
    ParseVar parseVar(const string &name, string &value);
    void parseHelp(const char *prefix);

    // DataSource interface:
    int getPathInfo(PathInfo **path_info);

    int getWaveforms(gvector<Waveform *> &wvec, bool displayed_only=true);
    Waveform *getWaveform(int id);
    int getComponents(vector<int> &num_cmpts, gvector<Waveform *> &wvec);
    int getSelectedWaveforms(gvector<Waveform *> &wvec);
    int getSelectedWaveforms(const string &cursor_label,
		gvector<Waveform *> &wvec);
    int copySelectedTimeSeries(gvector<GTimeSeries *> &tvec);
    void copyAllTables(DataSource *ds);
    bool dataWindowIsDisplayed(const string &cursor_label) {
	return doubleLineIsDisplayed(cursor_label);
    }
    int getSelectedComponents(const string &cursor_label, const string &comps,
		gvector<Waveform *> &wvec);
    int getSelectedComponents(const string &comps, gvector<Waveform *> &wvec);
    int getSelectedComponents(vector<int> &num_cmpts,gvector<Waveform *> &wvec);
    int getSelectedComponents(const string &cursor_label,
		vector<int> &num_cmpts, gvector<Waveform *> &wvec);
    int getArrivalsOnWaveform(Waveform *w, cvector<CssArrivalClass> &arrivals);

    void addDataListener(Component *comp) {
	addActionListener(comp, XtNdataChangeCallback);
    }
    void removeDataListener(Component *comp) {
	removeActionListener(comp, XtNdataChangeCallback);
    }
    bool selectRecord(CssTableClass *record, bool select, bool do_callback=false);
    int numSelected(void);
    bool empty(void);
    int numWaveforms(void);

    void putTable(CssTableClass *table, bool do_callback=true);

    void putArrivalWithColor(CssArrivalClass *arrival, Pixel fg) {
	putArrival(arrival, fg);
    }
    void putArrival(CssArrivalClass *arrival, Pixel fg) {
	change.arrival = true;
	if( CPlotPutArrival(arrival, fg) ) {
	    doDataChangeCallbacks();
	    TableListener::addListener(arrival, this);
	}
    }
    bool CPlotPutArrival(CssArrivalClass *arrival, Pixel fg);
    void modifyArrival(CssArrivalClass *a, bool do_callback=true) {
	moveArrival(a, a->time, false);
	renameArrival(a, a->phase);
	if(do_callback) {
	    change.arrival = true;
	    doDataChangeCallbacks();
	}
    }

    void doDataChangeCallbacks(void) {
	if(!hold_data_change_callbacks && dataChange()) {
	    doCallbacks(base_widget, (XtPointer)&change, XtNdataChangeCallback);
	    resetDataChange(false);
	}
    }
    void setCurveMargins(double left_margin, double right_margin,
		double bottom_margin, double top_margin);
    void zoomOnWaveforms(gvector<Waveform *> &wvec);
    static void addTableToAll(CssTableClass *table, CPlotClass *except_cp=NULL);

    bool isSelected(CssTableClass *table);
    bool selectTable(CssTableClass *table, bool select);
    bool retimeArrivalOn(CssArrivalClass *arrival, bool on);
    void retimeArrivalAllOff(void);
    void setReference();
    Waveform * getReference();

    static int getKeyActions(CPlotKeyAction **keys);
    static void setKeyActions(int num, CPlotKeyAction *keys);
    static void setDefaultKeyActions(void) {
	CPlotKeyAction *keys=NULL;
	int num_keys = CPlotGetDefaultKeyActions(&keys);
	CPlotSetKeyActions(num_keys, keys);
	Free(keys);
	putProperty("key_actions", "");
    }

    InfoArea *info_area;

    protected:
    // use these constructors in a "Widget" subclass (ie TtPlot, etc)
    CPlotClass(WidgetClass widget_class, const string &name, Component *parent,
		Arg *args, int n);
    CPlotClass(WidgetClass widget_class, const string &name, Component *parent,
		InfoArea *infoarea, Arg *args, int n);
    void setAppContext(XtAppContext app);
    void loadResources(void);
    void setParseArrival(CssArrivalClass *a);
    void setParseWaveform(Waveform *w);
    void setParseWaveforms(gvector<Waveform *> &ws);

    void removeArrival(CssArrivalClass *arrival);
    void removeOrigin(CssOriginClass *origin, bool do_callback);

    CPlotWidget	cw;
    MagnifyWindow *magnify_window;
    gvector<CssTableClass *> wfdiscs;

    private:

    static void singleSelectDataCallback(Widget, XtPointer, XtPointer);
    static void selectDataCallback(Widget, XtPointer, XtPointer);
    static void positionCallback(Widget, XtPointer,XtPointer);
    static void positionDragCallback(Widget, XtPointer, XtPointer);
    static void selectArrivalCallback(Widget, XtPointer, XtPointer);
    static void measureCallback(Widget, XtPointer, XtPointer);
    static void retimeCallback(Widget, XtPointer, XtPointer);
    static void retimeDragCallback(Widget, XtPointer, XtPointer);
    static void waveformMenuCallback(Widget, XtPointer,XtPointer);
    static void arrivalMenuCallback(Widget, XtPointer, XtPointer);
    static void waveformInfoCallback(Widget, XtPointer, XtPointer);
    static void arrivalInfoCallback(Widget, XtPointer, XtPointer);
    static void modifyWaveformCallback(Widget, XtPointer, XtPointer);
    static void rtdComputeCallback(Widget, XtPointer, XtPointer);
    static void magnifyCallback(Widget, XtPointer, XtPointer);
    static void menuCallback(Widget w, XtPointer client, XtPointer data);
    static void addArrivalCallback(Widget w, XtPointer client, XtPointer data);
    static void addArrivalMenuCallback(Widget w, XtPointer client, XtPointer data);
};

#endif
