#ifndef _FILTER_H
#define _FILTER_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"
#include "widget/Table.h"

class TaperData;
class TaperWindow;

typedef struct
{
    int		order;
    char	type[3];
    double	flow;
    double	fhigh;
    int		zero_phase;
    bool	unfilter;
    bool	replace;
    bool	all_waveforms;
    bool	use_taper;
} FilterStruct;

/**
 *  @ingroup libgx
 */
class Filter : public FormDialog, public DataReceiver
{
    public:
	Filter(const string &, Component *);
	Filter(const string &, Component *, DataSource *);
	~Filter(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd autoFilter(double low, double high, int order,
		const char *upper_type, bool zp);
	static void parseHelp(const char *prefix);

    protected:
	TaperWindow *taper_window;
	bool warn;
	FilterStruct filter;
	Waveform *parse_w;

	RowColumn *controls, *top_rc, *rc1, *rc2;
	Separator *sep;
	Choice *input_choice, *taper_choice, *mode_choice;
	Choice *order_choice, *type_choice, *zp_choice;
	Button *apply_button, *close_button, *unfilter_button, *save_button;
	Button *taper_width_button;
	Form *form, *form2;
	Button *more_button, *less_button;
	Label *low_label, *high_label;
	TextField *low_text, *high_text;
	Table *table;

	void init(void);
	void createInterface(void);
	void save(bool permanent=true);
	void setFilter(void);
	void apply(bool from_table);
	void unfilter(void);
	TaperData * getTaper(bool use_taper);
	void edit(MmTableEditCallbackStruct *c);
	void setSaveSensitive(void);
	void makePropertyString(string &prop);
	ParseCmd parseFilter(const string &cmd, string &msg);
};

#endif
