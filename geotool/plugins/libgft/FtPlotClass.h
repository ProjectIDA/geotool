#ifndef _FTPLOT_CLASS_H_
#define _FTPLOT_CLASS_H_

#include "widget/AxesClass.h"

#include "FtPlot.h"

namespace libgft {

class FtPlotClass : public AxesClass
{
    public:

	FtPlotClass(const char *name, Component *parent, Arg *args, int n);
	~FtPlotClass(void);

	void destroy(void);

	void initWidget(void);

	void clear(void) { FtPlotClear(ftplot); }
	void clearTmps(void) { FtPlotClearTmps(ftplot); }
	void compute(void) { FtPlotCompute(ftplot, true, true); }
	void doAverages(void) { FtPlotDoAverages(ftplot); }
	void ftWait(void) { FtPlotWait(ftplot); }
	void ftContinue(void) { FtPlotContinue(ftplot); }
/*
	void inputStatic(char *sta, char *chan, int nf, double df, float *y,
		int type, Pixel fg, double time) {
	    FtPlotInputStatic(ftplot, sta, chan, nf, df, y, type, fg, time);
	}
*/
	void input(const char *sta, const char *chan, int npts, float *y,
		double dt, Pixel fg, GTimeSeries *ts, double time)
	{
	    FtPlotInput(ftplot, sta, chan, npts, y, dt, fg, ts, time);
	}
/*
	void input2(char *sta, char *chan, int npts, float *y, double dt,
		int type, Pixel fg, double calper, double calib,
		vector<Response *> *rsp)
	{
	    FtPlotInput2(ftplot, sta, chan, npts, y, dt, type, fg, calper,
		calib, rsp, NULL);
	}
*/
	int getData(FtPlotData **ft_list, bool selected_only) {
	    return FtPlotGetData(ftplot, ft_list, (Boolean)selected_only);
	}
	void save(void) { FtPlotSave(ftplot); }
	void ftWrite(FILE *fp) { FtPlotWrite((Widget)ftplot, fp); }
	void ftRead(FILE *fp) { FtPlotRead((Widget)ftplot, fp); }
	bool writeEntry(int id, FILE *fp) {
	    return (bool)FtPlotWriteEntry(ftplot, id, fp);
	}
	bool readEntry(FILE *fp) {
	    return (bool)FtPlotReadEntry(ftplot, fp);
	}
	void fillSelected(void) { FtPlotFillSelected(ftplot); }
	void deleteSelected(void) { FtPlotDeleteSelected(ftplot); }
	void overlay(int npts, float *x, float *y, bool display) {
	    FtPlotOverlay(ftplot, npts, x, y, (Boolean)display);
	}
	int numSelected(void) { return FtPlotNumSelected(ftplot); }
	void smooth(double width) { FtPlotSmooth(ftplot, width); }
	double smoothingWidth(void) { return FtPlotSmoothingWidth(ftplot); }
	void setCosineTaper(double length, int min_pts, int max_pts) {
	    FtPlotSetCosineTaper(ftplot, length, min_pts, max_pts);
	}
	void getCosineTaper(double *length, int *min_pts, int *max_pts) {
	    FtPlotGetCosineTaper(ftplot, length, min_pts, max_pts);
	}
	int numEntries(void) {
	    return FtPlotNumEntries(ftplot);
	}

    protected:
	// use this constructor in a "Widget" subclass
	FtPlotClass(WidgetClass widget_class, const char *name,
		Component *parent, Arg *args, int n);

	FtPlotWidget	ftplot;

    private:

	static void selectCallback(Widget, XtPointer, XtPointer);
};

} // namespace libgft

#endif
