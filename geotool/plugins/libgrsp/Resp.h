#ifndef _RESP_H
#define _RESP_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "PrintClient.h"
#include "widget/PrintDialog.h"
#include "CSSTable.h"

/** @defgroup libgrsp plugin Instrument Response
 */

class DataSource;
class CPlotClass;
class Waveform;
class Response;
class ResponseFile;
class Working;
class TabClass;

namespace libgrsp {

class RespTapers;

/** Response window.
 *  @ingroup libgrsp
 */
class Resp : public Frame, public DataReceiver, public PrintClient
{
    public:
	Resp(const char *name, Component *parent, DataSource *ds);
	~Resp(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	ParseCmd parseConvolve(int direction, const string &s, string &msg);
	ParseCmd parseParams(const string &cmd, int *id, bool *instr,
			string &msg);
	void parseHelp(const char *prefix);

	virtual void print(FILE *fp, PrintParam *p);
	void setDataSource(DataSource *ds);

    protected:
	PrintDialog	*print_window;
	RespTapers	*resp_tapers;

	typedef struct {
	    Form *form;
	    ctable<CssInstrumentClass> *instrument_table;
	    Table *response_table;
	    vector<Response *> responses;
	    vector<ResponseFile *> response_files;
	    vector<int>	instrument_index;
	} RespStruct;

	RespStruct rps, all;

	bool ignore_data_change;

	// File menu
	Button *all_button, *print_button, *close_button, *open_button;

	// View menu
	Toggle *cascade_toggle, *grid_toggle;
	Menu *axes_menu;
	Toggle *log_xy_toggle, *log_x_toggle, *log_y_toggle, *linear_xy_toggle;
	Menu *display_menu;
	Toggle *amp_toggle, *power_toggle;

	// Option menu
	Button *convolve_button;
	Button *deconvolve_button;
	Button *original_button, *tapers_button;
	Toggle *remove_time_shift_toggle;
	Toggle *convert_disp_toggle;

	// Help menu
	Button *resp_help_button;

	CPlotClass *amp_plot, *phase_plot, *inverse_plot, *time_plot;
	Pane *pane;
	TabClass *tab, *table_tab;

	void init(void);
	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	void print(void);
	void listWaveformInstruments(void);
	void listResponses(RespStruct *rs);
	void listAllInstruments(void);
	void waveformResponse(RespStruct *rs, short adjust_limits);
	void addResponseRow(Table *table, Response *r, int row_num);
	void drawResponse(vector<Response *> &resp, Pixel fg, int selected,
		double samprate, double calper, double calib,
		short adjust_limits);
	void convolve(int direction);
	void conDecon(int direction, int num_resp, vector<int> &selected,
		gvector<Waveform *> &wvec);
	void removeMethods(gvector<Waveform *> &wvec);
	void open(void);
	void setButtonsSensitive(RespStruct *rs);

    private:

};

} // namespace libgrsp

#endif
