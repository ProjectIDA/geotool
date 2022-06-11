#ifndef _POLAR_FILTER_H
#define _POLAR_FILTER_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/ghashtable.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"

/** @defgroup polarFilter plugin Polarization Filter
 */

class WaveformPlot;
class TableSource;

namespace libghp {

/** Polar Filter window.
 *  @ingroup polarFilter
 */
class PolarFilter : public FormDialog, public DataReceiver
{
    public:
	PolarFilter(const char *, Component *, DataSource *);
	~PolarFilter(void);

	void setDataSource(DataSource *ds);
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	static int polarfilt(float *sz, float *se, float *sn, int npts,
		double dt, double tcycles, double fc1, double fc2,
		double fcycles, int norder, double apert, double rect,
		double thni, double phi, float *y);

    protected:
	WaveformPlot *wp;

	RowColumn *controls, *rc;
	Button *close_button, *apply_button, *help_button;
	Toggle *rtd_compute_toggle;
	Separator *sep;
	Label *label1, *label2, *label3, *label4, *label5, *label6, *label7;
	Label *label8, *label9;
	Scale *scale1, *scale2, *scale3, *scale4, *scale5, *scale6, *scale7;
	Scale *scale8, *scale9;
	TextField *order, *low_cut, *hi_cut, *freq_cycles, *time_cycles;
	TextField *azimuth, *incidence, *aperture, *recti_scaling;

	double ln_max, scale_hi_freq;
	ghashtable<string *> polar_prefix;

	void actionPerformed(ActionEvent *action_event);
	void createInterface(void);
	void scale(Scale *scale);
	double getMaxNyquist();
	void apply(void);

    private:

};

} // namespace libghp

#endif
