#ifndef WAVEFORM_VIEW_H
#define WAVEFORM_VIEW_H

#include "WaveformPlot.h"
#include "motif++/MotifDecs.h"
#include "gobject++/CssTables.h"
#include "widget/MmTable.h"

extern "C" {
#include "crust.h"
}

class Waveform;
class UndoEditView;
class UndoDeleteView;
class WaveformWindow;
class InfoArea;
class CSSTable;
class CutData;

/**
 *  @ingroup libgx
 */
class WaveformView : public WaveformPlot
{
    friend class WaveformWindow;
    friend class UndoEditView;
    friend class UndoDeleteView;

    public:

	WaveformView(const string &name, Component *parent, Arg *args=NULL,
			int num=0);
	WaveformView(const string &, Component *, InfoArea *, Arg *, int);
	WaveformView(const string &, Component *, Arg *, int, DataSource *ds);
	WaveformView(const string &, Component *, InfoArea *, Arg *, int,
			DataSource *ds);
	~WaveformView(void);

	void actionPerformed(ActionEvent *action_event);
	void pasteData(void);
	void pasteData(gvector<GTimeSeries *> &data);

	virtual WaveformView *getWaveformViewInstance(void) { return this; }
	void renameApply(CssArrivalClass *a, const string &new_name);

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd parseCopyTable(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:

	CPlotMenuCallbackStruct cplot_menu;

	PopupMenu *arrival_popup;
	Button *arrival_rename;
	Button *arrival_retime;
	Button *arrival_beam;
	Button *arrival_fk;
	Button *arrival_fk_mb;
	Button *arrival_polar;
	Button *arrival_ml_amp;
	Button *arrival_mb_amp;
	Button *arrival_delete;

	FormDialog	*rename_arrival_popup;
	TextField	*rename_arrival_text;
	FormDialog	*retime_arrival_popup;
	TextField	*retime_arrival_text1;
	TextField	*retime_arrival_text2;
	TextField	*retime_arrival_text3;
	cvector<CssArrivalClass> retime_arrival;
	cvector<CssOriginClass>	 retime_origin;
	char	reset_rename[20];
	double	reset_retime;

	PopupMenu *waveform_popup;
	Menu	*calib_menu;
	Toggle	*calib_apply;
	Toggle	*calib_remove;
	Toggle	*demean_toggle;
	Toggle	*polarity_toggle;
	Menu	*filter_menu;
	Button	*unfilter_button;
	Button	*copy_one_button, *copy_button;
	Button	*cut_one_button, *cut_button;
	Button	*paste_button;
	Button	*display_button;
	Button	*elements_button;
	Button	*zoom_button;

	CSSTable *paste_table;
	vector <WaveformWindow *> windows;

	typedef struct
	{
	    Button  *b;
	    double  flo;
	    double  fhi;
	    int     order;
	    char    type[3];
	    int     zero_phase;
	} FilterButton;

#define MAX_FILTER 20

	int num_filter_buttons;
	FilterButton filter_buttons[MAX_FILTER];

	void init(const string &name, Component *parent, Arg *args, int n);

	virtual void arrivalMenu(CPlotMenuCallbackStruct *c);
	virtual void arrivalInfo(CPlotInfoCallbackStruct *c);
	virtual void waveformMenu(CPlotMenuCallbackStruct *c);
	virtual void waveformInfo(CPlotInfoCallbackStruct *c);

	void editTimeText(TextField *comp);
	void printTime(CssArrivalClass *a);
	void retimeArrival(void);
	void retimeApply(void);
	void retimeCancel(void);
	void renameArrivalCB(void);
	void renameApply(const string &new_name);
	void renameCancel(void);
	void deleteArrivalCB(void);

	void calibApply(bool apply);
	void demean(bool apply);
	void changePolarity(void);
	void copyWaveforms(void);
	void copyWaveforms(gvector<Waveform *> &wvec,
		double tmin=NULL_TIME, double tmax=NULL_TIME) {
	    cutOrCopy(wvec, false, tmin, tmax, false);
	}
	void cutWaveforms(void);
	void cutWaveforms(gvector<Waveform *> &wvec, bool delete_data=false,
		double tmin=NULL_TIME, double tmax=NULL_TIME) {
	    cutOrCopy(wvec, delete_data, tmin, tmax, true);
	}
	void cutOrCopy(gvector<Waveform *> &wvec, bool delete_data,
		double tmin, double tmax, bool cut);
	int numClipBoard(void);

	void filter(Button *button);
	void unfilter(void);
	void displayWaveform(void);
	void displayElements(void);
	virtual void makePopupMenu(void);
	bool undoEditArrival(UndoEditView *undo);
	ParseCmd parseDisplayWaveform(const string &cmd, string &msg);
	PopupMenu *createArrivalPopup(void);
	PopupMenu *createWaveformPopup(void);

    private:
	static void destroyRetimeCB(Widget, XtPointer, XtPointer);
};

#endif
