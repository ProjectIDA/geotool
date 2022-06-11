#ifndef _AMPLITUDE_PARAMS_H
#define _AMPLITUDE_PARAMS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"
#include "FKData.h"

class TabClass;
/** 
 *  @ingroup libgx
 */
class AmplitudeParams : public FormDialog
{
    public:
	AmplitudeParams(const string &, Component *);
	~AmplitudeParams(void);

	void actionPerformed(ActionEvent *action_event);
	void setVisible(bool visible) {
	    if(visible) list();
	    this->FormDialog::setVisible(visible);
	}
	ParseCmd parseCmd(const string &cmd, string &msg);

	bool auto_measure;

	string mb_amptype;	//!<
	double mb_filter_margin; //!<
	double mb_lead; //!<
	double mb_length; //!<
	double mb_taper_frac; //!<

	string mb_filter_type; //! The amplitude filter type.
	int mb_filter_order; //!<
	double mb_filter_locut; //!<
	double mb_filter_hicut; //!<
	bool mb_filter_zp; //!<

	double mb_amp_threshold1; //!<
	double mb_amp_threshold2;	//!<
	double mb_amp_threshold3; //!<
	double mb_amp_threshold4; //!<
	double mb_amp_threshold5; //!<
	double mb_amp_threshold6; //!<
	double allowed_hp_ratio; //!<
	double allowed_lp_ratio; //!<
	bool mb_allow_counts;
	string mb_counts_amptype;	//!<

	int mb_num_phases;
	char **mb_phases;
	double mb_dist_min;
	double mb_dist_max;

	string ml_amptype;	//!<
	int ml_num_phases;
	char **ml_phases;
	double ml_dist_min;
	double ml_dist_max;
	double ml_depth_min;
	double ml_depth_max;
	double ml_sta_lead;
	double ml_sta_window;
	double ml_sta_length;
	double ml_lta_lead;
	double ml_lta_length;
	double ml_filter_margin; //!<
	string ml_filter_type; //! The amplitude filter type.
	int ml_filter_order;
	double ml_filter_locut;
	double ml_filter_hicut;
	bool ml_filter_zp;

    protected:
	TabClass	*tab;
	RowColumn	*controls;
	Button		*close_button, *save_button, *defaults_button;
	Button		*more_button, *less_button, *help_button;
	Toggle		*auto_measure_toggle;
	Separator	*sep, *sep2;
	Label		*label, *label2;
	Table		*a5_table, *sbsnr_table;
	RowColumn	*rc;
	int		edit_file;

	bool auto_measure_default;
	string mb_amptype_default;
	double mb_filter_margin_default;
	double mb_lead_default;
	double mb_length_default;
	double mb_taper_frac_default;

	string mb_filter_type_default;
	int mb_filter_order_default;
	double mb_filter_locut_default;
	double mb_filter_hicut_default;
	bool mb_filter_zp_default;

	double mb_amp_threshold1_default;
	double mb_amp_threshold2_default;
	double mb_amp_threshold3_default;
	double mb_amp_threshold4_default;
	double mb_amp_threshold5_default;
	double mb_amp_threshold6_default;
	double allowed_hp_ratio_default; 
	double allowed_lp_ratio_default;
	bool mb_allow_counts_default;
	string mb_counts_amptype_default;

	int mb_num_phases_default;
	char **mb_phases_default;
	double mb_dist_min_default;
	double mb_dist_max_default;

	string ml_amptype_default;
	int ml_num_phases_default;
	char **ml_phases_default;
	double ml_dist_min_default;
	double ml_dist_max_default;
	double ml_depth_min_default;
	double ml_depth_max_default;
	double ml_sta_lead_default;
	double ml_sta_window_default;
	double ml_sta_length_default;
	double ml_lta_lead_default;
	double ml_lta_length_default;
	double ml_filter_margin_default;
	string ml_filter_type_default;
	int ml_filter_order_default;
	double ml_filter_locut_default;
	double ml_filter_hicut_default;
	bool ml_filter_zp_default;

	void createInterface(void);
	void init(void);
	void list(void);
	void listA5(void);
	void listSbsnr(void);
	void save(void);
	bool getA5Params(vector<const char *> &col, bool set, bool *change,
			bool *defaults);
	bool getSbsnrParams(vector<const char *> &col, bool set, bool *change,
			bool *defaults);
	void setButtonsSensitive(void);
	void setA5Defaults(void);
	void setSbsnrDefaults(void);

    private:
};

#endif
