#ifndef _CREATE_SLOWNESS_H
#define _CREATE_SLOWNESS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "WaveformPlot.h"
#include "CSSTable.h"

class Table;
class MultiTable;
class TabClass;
class Iaspei;

class CreateBeam : public FormDialog
{
    public:
	CreateBeam(const string &, Component *, WaveformPlot *);
	~CreateBeam(void);

	TextField *slownessText1(void) { return slowness_text1; };
	TextField *slownessText2(void) { return slowness_text2; };
	TextField *azimuthText1(void) { return azimuth_text1; };
	TextField *azimuthText2(void) { return azimuth_text2; };
	TextField *wlenText(void) { return window_text; };
	TextField *snrText(void) { return snr_text; };
 	void setLow(double low) {
	    char s[20];
	    snprintf(s, sizeof(s), "%.15g", low);
	    table->setField(0, 0, s, true);
	}
 	void setHigh(double high) {
	    char s[20];
	    snprintf(s, sizeof(s), "%.15g", high);
	    table->setField(0, 1, s, true);
	}
 	void setOrder(int order) {
	    char s[20];
	    snprintf(s, sizeof(s), "%d", order);
	    table->setField(0, 2, s, true);
	}
 	void setType(const string &type) {
	    table->setField(0, 3, type, true);
	}
 	void setConstraint(bool zp) {
	    if(zp) table->setField(0, 4, "zero phase", true);
	    else table->setField(0, 4, "causal", true);
	}
	void setKm(void) { s_km->set(true, true); }
	void setDeg(void) { s_deg->set(true, true); }
	bool replace(void) { return replace_toggle->state(); }
	void setReplace(bool b) { replace_toggle->set(b); }
	void useFilter(bool b) {
	    if(b) filter_on->set(true, true);
	    else filter_off->set(true, true);
	}
	void setWaveformPlot(WaveformPlot *wplot) {
	    if(wp != wplot) {
		wp->removeDataListener(this);
		wp = wplot;
		wp->addDataListener(this);
	    }
	}
	virtual bool makeBeam(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	TabClass *tab;

    protected:
	RowColumn *input_rc, *input_rc2, *input_rc3, *input_rc4;
	RowColumn *origin_rc, *origin_rc2, *arrival_rc;
	RowColumn *controls, *filter_rc;
	Separator *sep1, *sep2;
	Button *close_button, *help_button, *compute_button, *align_button;
	Label *slowness_label, *azimuth_label, *waveform_label;
	Label *label1, *label2, *label3, *label4, *label5, *label6,*phase_label;
	TextField *slowness_text1, *azimuth_text1;
	TextField *slowness_text2, *azimuth_text2;
	TextField *window_text, *snr_text;
	RadioBox *rb, *rb2, *rb3;
	Toggle *s_km, *s_deg, *filter_on, *filter_off, *replace_toggle;
	Toggle *beam_toggle, *ftrace_toggle;
	Table *table;
	Form *form, *hide_form;
	ScrolledWindow *sw;
	List *phase_list;
	Toggle *recipe_waveforms, *selected_waveforms;
	MultiTable *origin_table;
	ctable<CssArrivalClass> *arrival_table;
	WaveformPlot *wp;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void setSensitive(void);
	bool getFilter(char *type, int *order, double *flo, double *fhi,
		int *zero_phase);
	void initPhaseList(void);
	void initOriginTable(void);
	void listOrigins(void);
	void listArrivals(void);
	virtual bool getAzimuth(double *azimuth);
	virtual bool getSlowness(double *slowness);
	void computeSlowAz(void);

    private:

};

#endif
