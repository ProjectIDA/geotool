#ifndef _TTPLOT_CLASS_H_
#define _TTPLOT_CLASS_H_

#include "widget/CPlotClass.h"
#include "widget/TtPlot.h"

/** This class is the interface to the TtPlot Widget.
 *  @ingroup libwgets
 */
class TtPlotClass : public CPlotClass
{
    public:

	TtPlotClass(const string &name, Component *parent, Arg *args, int n) :
		CPlotClass(ttPlotWidgetClass, name, parent, args, n)
	{
	    tw = (TtPlotWidget)base_widget;
	}
	TtPlotClass(const string &name, Component *parent, InfoArea *infoarea,
		Arg *args, int n) :
		CPlotClass(ttPlotWidgetClass, name, parent, infoarea, args, n)
	{
	    tw = (TtPlotWidget)base_widget;
	}
	~TtPlotClass(void) {}

	void initWidget(void);

	void setComputeTTMethod(ComputeTTMethod method);
	double getTravelTime(const string &phase, CssOriginClass *origin, double lat,
		double lon, double elev, const string &net, const string &sta,
		double *slowness, string &op);
	bool firstArrival(CssOriginClass *origin, double lat, double lon,
		double elev, const string &net, const string &sta, char type,
		string &phase, double *time, double *slowness);
	void crust(CrustModel *crust_model);

	virtual Waveform *addTimeSeries(GTimeSeries *ts,
			CPlotInputStruct *input);
	void changePredArr(int n, InputPredArr *in_pred_arr, int action);
	int alignOnPredictedPhase(const string &phase);
	virtual bool getDataDuration(double *tmin, double *tmax);
	virtual void clear(bool do_callback=true);
	virtual void clearWaveforms(bool do_callback=true);
	virtual void deleteWaveforms(gvector<Waveform *> &wvec);
	virtual void deleteWaveforms_NoReposition(gvector<Waveform *> &wvec);
	virtual void deleteSelectedWaveforms(void);
	virtual void setTimeLimits(double tmin, double tmax);
	virtual void positionY(gvector<Waveform *> &wvec,
			vector<double> &scaled_y0);
	void setWaveformOrder(gvector<Waveform *> &wvec, bool redisplay=true);
	virtual void displayComponents(int display);
	virtual void setDataHeight(int dataHeight, int dataSeparation);
	virtual void setDataDisplay(gvector<Waveform *> &wvec,
                        vector<bool> &on, bool promote_visible, bool redisplay);
	int getIaspeiPhases(vector<const char *> &phase_list);
	int getCurve(const string &phase, int *npts, float *tt, float *dist,
			float *ray_p);
	void setSourceDepth(double depth);
	double getSourceDepth(void);
	void updateForOriginChange(void);
	void setReductionVelocity(double velocity, bool reduced_time);
	void setCelerity(double celerity);
	void useCelerity(bool use_celerity);
	void selectIaspeiPhases(bool select);
	void updatePredicted(void);
	virtual void setWorkingOrid(long orid, bool do_callback=true) {
	    CPlotClass::setWorkingOrid(orid, false);
	    updatePredicted();
	    change.working_orid = true;
	    if(do_callback) doDataChangeCallbacks();
	}

    protected:
	// use this constructor in a "Widget" subclass
	TtPlotClass(WidgetClass widget_class, const string &name,
			Component *parent, Arg *args, int n) :
		CPlotClass(widget_class, name, parent, args, n)
	{
	    tw = (TtPlotWidget)base_widget;
	}

	TtPlotWidget	tw;

    private:
};

#endif
