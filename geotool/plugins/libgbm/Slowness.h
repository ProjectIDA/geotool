#ifndef _SLOWNESS_H
#define _SLOWNESS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/DataSource.h"

class WaveformPlot;
class Table;
class MultiTable;
class TabClass;
class TravelTime;

namespace libgbm {

class Slowness : public FormDialog
{
    public:
	Slowness(const char *, Component *, DataSource *);
	~Slowness(void);

    protected:
	RowColumn *input_rc, *rc1, *rc2, *rc3, *controls, *origin_rc;
	Separator *sep1, *sep2;
	Button *close_button, *help_button, *beam_button, *align_button;
	Label *label1, *label2, *label3, *label4, *phase_label;
	TextField *slowness_text1, *azimuth_text1;
	TextField *slowness_text2, *azimuth_text2;
	RadioBox *rb, *rb2;
	Toggle *s_km, *s_deg, *filter_on, *filter_off, *replace_toggle;
	Table *table;
	TabClass *tab;
	Form *form;
	ScrolledWindow *sw;
	List *phase_list;
	MultiTable *origin_table;
	WaveformPlot *wp;
	TravelTime *travel_time;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void setSensitive(void);
	bool makeBeam(void);
	bool getFilter(char *type, int *order, double *flo, double *fhi,
		int *zero_phase);
	void initPhaseList(void);
	void initOriginTable(void);
	void list(void);
	bool getAzimuth(double *azimuth);
	bool getSlowness(double *slowness);
	void computeSlowAz(void);

    private:

};

} // namespace libgbm

#endif
