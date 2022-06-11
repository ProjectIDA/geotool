#ifndef _CALIBRATION_H
#define _CALIBRATION_H

#include "motif++/MotifDecs.h"
#include "WaveformWindow.h"

/** @defgroup libgcal plugin Calibration
 */
class Table;

struct CalibParam_s;
struct CalibOut_s;

namespace libgcal {

class CalParam;
class Iteration;

/** Calibration window.
 *  @ingroup libgcal
 */
class Calibration : public WaveformWindow
{
    public:
	Calibration(const char *, Component *);
	~Calibration(void);

	bool readParFile(const string &file);
	void compute(void);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	// File menu
	Button *compute_button, *open_button;

	// View menu
	Button *align_button;

	// Option menu
	Button *parameters_button, *interation_button;

	// Help menu
	Button *cal_help_button;

	Table *table;
	CalParam *cal_param_window;
	Iteration *iteration_window;
	struct CalibParam_s *p;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *action_event);
	void printWindow(void);
	void displayData(double *in, double *out, struct CalibOut_s *co);
	void open(void);
	void putInterfaceParams(void);
	bool getInterfaceParams(void);

    private:

};

} // libgcal

#endif
