#ifndef _MEASURE_AMP_PER_H
#define _MEASURE_AMP_PER_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "widget/CPlotClass.h"

class DataSource;
class Table;
class Waveform;
class CPlotClass;

namespace libgarrival {

class Arrivals;
class SearchParam;

class MeasureAmpPer : public Frame, public DataReceiver
{
    friend class Arrivals;

    public:
	MeasureAmpPer(const char *, Component *, DataSource *, CPlotClass *cp);
	~MeasureAmpPer(void);

	void setVisible(bool visible);

    protected:
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	SearchParam	*search_param_window;
	CPlotClass	*cp;
	gvector<Waveform *> ws;
	string		amp_type;

	// File menu
	Button *close_button;

	// Edit menu
	Button *save_all_button, *save_selected_button, *create_amp_button;
	Toggle *net_toggle;

	// View menu
	Button *attributes_button, *clear_selected_button, *clear_all_button;

	// Option menu
	Menu *dfx_menu;
	Button *peak_trough_button, *zero_peak_button, *fm_a_button;
	Button *fm_b_button, *fm_button, *fm_sign_button, *rms_button;
	Button *mean_sqr_button, *abs_max_button, *avg_max_button, *stav_button;
	Button *measure_all_button, *measure_sel_button;
	Toggle *dfx_interp_toggle;
	Button *search_button, *search_param_button;
	Toggle *review_toggle;

	// Help menu
	Button *help_button;

	Table *table;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *a);
	void list(void);
	void measurement(CPlotMeasure *m);
	void getRow(CPlotMeasure *m, int num_columns, vector<string> &row);
	void clearBoxes(void);
	void save(bool selected_only);
	int getMeasuredArrivals(vector<CssArrivalClass *> &arr, bool selected_only);
	void changeAmpPer(vector<CssArrivalClass *> &arrivals);
	void createAmplitudes(void);
	void measure(bool selected);
	void search(void);
	void DFXMeasureAmp(const char *type);

    private:
};

} // namespace libgarrival

#endif
