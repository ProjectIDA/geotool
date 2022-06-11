#ifndef _SPECTROPARAM_H
#define _SPECTROPARAM_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"
#include "motif++/Toggle.h"

namespace libgspectro { 

typedef struct
{
	char	*lo_freq_text;
	char	*hi_freq_text;
	char	*window_length_text;
	char	*window_overlap_text;
} SpectroParamStruct;

class SpectroParam : public ParamDialog
{
    public:
	SpectroParam(const char *name, Component *parent);
	~SpectroParam(void);

	TextField *loFreqWidget(void) { return lo_freq_text; }
	TextField *hiFreqWidget(void) { return hi_freq_text; }
	TextField *windowLengthWidget(void) { return window_length_text; }
	TextField *windowOverlapWidget(void) { return window_overlap_text; }
	bool autoState(void) { return auto_toggle->state(); }
	void ignoreTextInput(bool set) { ignore_set_text = set; }
	ParseCmd parseCmd(const string &cmd, string &msg);

    protected:

	Button		*compute_button;
	Toggle		*auto_toggle;
	TextField	*lo_freq_text;
	TextField	*hi_freq_text;
	TextField	*window_length_text;
	TextField	*window_overlap_text;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void compute(void);

    private:
	bool		ignore_set_text;
};

} // namespace libgspectro

#endif
