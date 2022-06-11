#ifndef _FK_PARAM_DIALOG_H
#define _FK_PARAM_DIALOG_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"
#include "FK.h"

namespace libgfk {

typedef struct
{
	char	*s_max_text;
	char	*num_s_text;
	char	*slow_min_text;
	char	*slow_max_text;
	char	*az_min_text;
	char	*az_max_text;
} FkParamDialogStruct;

class FkParamDialog : public ParamDialog
{
    public:
	FkParamDialog(const char *name, Component *parent, FKType type);
	~FkParamDialog(void);

	ParseCmd parseCmd(const string &cmd, string &msg);

	Label *slowMaxLabel(void) { return s_max_label; }

	static void parseHelp(const char *prefix);

	TextField	*s_max_text;
	TextField	*num_s_text;
	TextField	*slow_min_text;
	TextField	*slow_max_text;
	TextField	*az_min_text;
	TextField	*az_max_text;
	TextField	*window_length_text;
	TextField	*window_overlap_text;
	TextField	*bandwidth_text;
	TextField	*stav_length_text;
	TextField	*ltav_length_text;
	Toggle		*scan_frequencies_toggle;
	Label		*bandwidth_label;

    protected:

	Button		*close_button, *compute_button, *help_button;
	Separator	*sep;
	RowColumn	*rc;
	Label		*s_max_label, *num_s_label;
	Label		*slow_min_label, *slow_max_label;
	Label		*az_min_label, *az_max_label;
	Label		*window_length_label, *window_overlap_label;
	Label		*stav_length_label, *ltav_length_label;

	void createInterface(FKType type);
	void actionPerformed(ActionEvent *a);
	void compute(void);

    private:
};

} // namespace libgfk

#endif

