#ifndef _ARRIVAL_PARAMS_H
#define _ARRIVAL_PARAMS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"
#include "FKData.h"

/** 
 *  @ingroup libgx
 */
class ArrivalParams : public FormDialog
{
    public:
	ArrivalParams(const string &, Component *);
	~ArrivalParams(void);

	void actionPerformed(ActionEvent *action_event);
	void setVisible(bool visible) {
	    if(visible) list();
	    this->FormDialog::setVisible(visible);
	}
        ParseCmd parseCmd(const string &cmd, string &msg);

	double stav_len; //!< the short term average window length (secs)
	double ltav_len; //!< the long term average window length (secs)
	double min_snr; //!< the minimum signal to noise ratio.
	double max_snr; //!< the maximum signal to noise ratio.
	double min_deltim; //!< the minimum deltim value.
	double max_deltim; //!< the maximum deltim value.

	double fk_lead; //!< the fk window starts at arrival.time - fk_lead secs
	double fk_lag; //!< the fk window ends at arrival.time + fk_lag secs
	double fk_dk; //!< Fk dk value used only in the computation of delslo.
	double signal_slow_min;	//!< the minimum slowness considered for fstat
	double signal_slow_max;	//!< the maximum slowness considered for fstat
	double signal_az_min; //!< the minimum azimuth considered for fstat
	double signal_az_max; //!< the maximum azimuth considered for fstat
	int num_bands; //!< the number of frequency bands to compute.
	double fmin[MAX_NBANDS]; //!< the frequency band minimum frequency.
	double fmax[MAX_NBANDS]; //!< the frequency band maximum frequency.
	int fk_taper_type; //!< the taper type.
	/** the fraction of the data window tapered (Cosine taper only) */
	double fk_taper_frac;

	double polar_signal_lead;
	double polar_signal_len;
	double polar_window;
	double polar_alpha;
	double polar_dk;

	bool polar_zp;
	double polar_lofreq;
	double polar_hifreq;
	int polar_order;
	double polar_taper_frac;
	string polar_filter_type;
	string arrival_beam_recipe;

	double single_fmin;
	double single_fmax;
	double search_fmin;
	double search_fmax;
	double search_bandw;
	bool search_bands;

    protected:
	RowColumn	*controls;
	Button		*close_button, *save_button, *help_button;
	Button		*defaults_button, *more_button, *less_button;
	Separator	*sep, *sep2;
	Label		*label, *label2;
//	Table		*table, *fband_table;
	Table		*table;
	RowColumn	*rc;
	Toggle		*single_band_toggle, *search_bands_toggle;
	Label		*single_fmin_label, *single_fmax_label;
	Label		*search_fmin_label, *search_fmax_label;
	Label		*search_bandw_label;
	TextField	*single_fmin_text, *single_fmax_text;
	TextField	*search_fmin_text, *search_bandw_text,*search_fmax_text;
	int		edit_file;

	double stav_len_default;
	double ltav_len_default;
	double min_snr_default;
	double max_snr_default;
	double min_deltim_default;
	double max_deltim_default;

	double fk_lead_default;
	double fk_lag_default;
	double fk_dk_default;
	double signal_slow_min_default;
	double signal_slow_max_default;
	double signal_az_min_default;
	double signal_az_max_default;
	int num_bands_default;
	double fmin_default[MAX_NBANDS];
	double fmax_default[MAX_NBANDS];
	int fk_taper_type_default;
	double fk_taper_frac_default;

	double polar_signal_lead_default;
	double polar_signal_len_default;
	double polar_window_default;
	double polar_alpha_default;
	double polar_dk_default;

	bool polar_zp_default;
	double polar_lofreq_default;
	double polar_hifreq_default;
	int polar_order_default;
	double polar_taper_frac_default;
	string polar_filter_type_default;
	string arrival_beam_recipe_default;

	double single_fmin_default;
	double single_fmax_default;
	double search_fmin_default;
	double search_fmax_default;
	double search_bandw_default;
	bool search_bands_default;

	void createInterface(void);
	void init(void);
	void initBands(void);
	void list(void);
	void save(void);
	bool getParams(vector<const char *> &col, bool set, bool *change,
			bool *defaults);
	void setButtonsSensitive(void);
	void setDefaults(void);

    private:
};

#endif
