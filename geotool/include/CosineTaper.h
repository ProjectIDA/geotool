#ifndef _COSINE_TAPER_H
#define _COSINE_TAPER_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"


enum CosineTaperUnits
{
    PERCENTAGE,
    SECONDS,
    SAMPLES
};

/**
 *  @ingroup libgx
 */
class CosineTaper : public ParamDialog
{
    public:
	CosineTaper(const string &name, Component *parent);
	~CosineTaper(void);

	void actionPerformed(ActionEvent *action_event);

	CosineTaperUnits getBegTaper(double *value);
	CosineTaperUnits getEndTaper(double *value);
	void setBegTaper(CosineTaperUnits units, double value);
	void setEndTaper(CosineTaperUnits units, double value);

	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:

	RowColumn *controls, *rc;
	Separator *sep;
	Button	*close_button;
	Label *lab1, *lab2, *lab3;
	Form *form1, *form2, *form3, *form4;
	Scale	*beg_scale, *end_scale;
	TextField *beg_seconds_text, *end_seconds_text;
	TextField *beg_samples_text, *end_samples_text;

	void createInterface(void);

    private:
};

#endif
