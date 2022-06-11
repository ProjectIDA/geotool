#ifndef _AMP_MAG_H
#define _AMP_MAG_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "Beam.h"
#include "WaveformPlot.h"
#include "CSSTable.h"
#include "gobject++/gvector.h"

class DataSource;
class WaveformView;
class GTimeSeries;
class ReviewSource;

namespace libgarrival {

class MeasureAmpPer;

class AmpMag : public Frame, public DataReceiver
{
    public:
	AmpMag(const char *, Component *, DataSource *, WaveformPlot *);
	~AmpMag(void);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);

    protected:
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	MeasureAmpPer	*measure_window;
	WaveformPlot	*wp;
	ReviewSource	*review_source;
	Beam		*gbeam;

	// File menu
	Button *print_button, *close_button;

	// Edit menu
	Button *edit_button, *cancel_button, *delete_button;
	Button *filter_button, *save_button;
	UndoButton *undo_button;

	// View menu
	Button *amp_attributes_button, *stamag_attributes_button;
	Button *netmag_attributes_button;
	Button *deselect_button;
	Menu *sort_menu;
	Button *amp_sort_button, *stamag_sort_button, *netmag_sort_button;
	Button *sort_cols_button;

	// Option menu
	Button *compute_button, *measure_ml_button, *measure_mb_button,
		*manual_button, *review_button;

	// Help menu
	Button *help_button;

	Pane *pane;
	ctable<CssAmplitudeClass> *amp_table;
	ctable<CssStamagClass> *stamag_table;
	ctable<CssNetmagClass> *netmag_table;
	WaveformView *plot1;
	bool ignore_measure_cb, force_edit_on;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *a);
	void selectRow(CSSTable *table);
	void list(void);
	void amplitudeList(void);
	void stamagList(void);
	void netmagList(void);
	void computeMagnitudes(void);
	void measureAmplitude(AmpMeasureMode mode);
	void initMagLib(CssTableClass *css);
	void updateTables(void);
	void saveMagnitudes(void);
	void removeNegMagids(void);
	void deleteSelected(void);
	int getSelectedAmps(cvector<CssAmplitudeClass> &amplitudes);
	int getSelectedNetmags(cvector<CssNetmagClass> &netmags);
	int getSelectedStamags(cvector<CssStamagClass> &stamags);
	void amplitudeReview(void);
	void review(vector<bool> &states, cvector<CssAmplitudeClass> &amps,
		cvector<CssArrivalClass> &arrivals);
	void reviewBeamAmp(gvector<Waveform *> &wvec,
		Waveform *main_w, CssAmplitudeClass *gamp, CssArrivalClass *garr,
		BeamRecipe &recipe, double *tmin, double *tmax,
		double *max_duration);
	int getCDs(vector<BeamSta> &beam_sta, gvector<Waveform *> &wvec,
		gvector<Waveform *> &ws);
	void loadTs(GTimeSeries *ts, CssAmplitudeClass *gamp, CssArrivalClass *garr,
		CssAssocClass *assoc, Waveform *main_w,
		double *tmin, double *tmax, double *max_duration);
	void reviewRegularAmp(Waveform *main_w, CssAmplitudeClass *gamp,
		CssArrivalClass *garr, double *tmin, double *tmax,
		double *max_duration);
	void getColumns(int *amp_col, int *per_col, int *amptime_col,
		int *start_time_col);
	void ampMeasure(CPlotMeasure *m);
	void editOn(void);
	void editOff(bool save);

	typedef struct
	{
	    char	amp_format[20];
	    char	per_format[20];
	    char	amptime_format[20];
	    char	start_time_format[20];
	} AmpFormats;

	void updateAmpTable(AmpFormats *formats, CssAmplitudeClass *gamp,
		CPlotMeasure *m, GTimeSeries *ts);
	void getAmpFormats(AmpFormats *formats);
	Pixel nextColor();
	void updateAmpPlot(void);
	void cancelAmplitudes(void);
	void modifyReview(CPlotModifyCallbackStruct *c);
};

} // namespace libgarrival

#endif
