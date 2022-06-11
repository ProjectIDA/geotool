#ifndef _GCEPSTRUM_H
#define _GCEPSTRUM_H

#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "motif++/Frame.h"

/** @defgroup libgcepstrum plugin Cepstrum
 */

class TabClass;
class CPlotClass;
class DataSource;
struct CepstrumParam_s;

namespace libgcepstrum {

class CepstrumParams;

/** Cepstrum window.
 *  @ingroup libgcepstrum
 */
class GCepstrum : public Frame, public DataReceiver, public PrintClient
{
    public:
	GCepstrum(const char *, Component *, DataSource *);
	~GCepstrum(void);

	virtual void print(FILE *fp, PrintParam *p);
	void compute(void);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	bool parseSetParam(const string &cmd);
	void parseHelp(const char *prefix);

    protected:
	// File menu
	Button *compute_button, *print_button, *new_cep_button, *close_button;
	Menu *cursor_menu;
	Toggle *cursor_toggle[4];

	// Option menu
	Button *parameters_button;

	// Help menu
	Button *cepstrum_help_button;

	TabClass *tab;
	Form *form[6];
	CPlotClass *plot[6];

	CepstrumParams *param_window;
	PrintDialog *print_window;

	vector<GCepstrum *> windows;

	typedef struct
	{
	    char	sta[7];
	    char	chan[9];
	    double	time;
	    int		nf;
	    int		wfid;
	    double	df;
	    double	dt;
	    char	taper[9];
	    int		taperstart;
	    int		taperend;
	    int		npts;
	    int		signal_type;
	    double	smoothvalue;
	    float	*data;
	} CepstrumStruct;

	char compute_tab[100];

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *action_event);
	void print(void);
	int getData(CepstrumStruct *cs);
	bool getParam(struct CepstrumParam_s *cp);
	void selectCursor(Toggle *toggle);
	void cepstrumLimits(CPlotClass *cp, AxesCursorCallbackStruct *p);
	void setText(char *label, double x);
	void frequencyLimits(char *name);
	void drawTrendLine(CPlotClass *cplot, double *f, float *data, int nf,
		bool add);
	void selectTab(char *label);

    private:

};

} // namespace libgcepstrum

#endif
