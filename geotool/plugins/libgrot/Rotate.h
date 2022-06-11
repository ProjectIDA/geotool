#ifndef _ROTATE_H
#define _ROTATE_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "gobject++/DataSource.h"
#include "widget/DialClass.h"
#include "Waveform.h"
#include "gobject++/GTimeSeries.h"

/** @defgroup libgrot plugin Rotation
 */

class BasicMap;
class Iaspei;

namespace libgrot {

/** Rotation window.
 *  @ingroup libgrot
 */
class Rotate : public FormDialog, public DataReceiver
{
    public:
	Rotate(const char *, Component *, DataSource *);
	~Rotate(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	char map_sta[10];
	double sta_lat, sta_lon, map_azimuth, map_distance;
	double sta_to_src_azimuth, reverse_azimuth, distance, incidence;
	BasicMap *map;

	RowColumn *controls, *rc, *dial_rc, *rc2;
	Button *close_button, *unrotate_button, *save_button, *help_button;
	Button *apply_button;
	Separator *sep;
	Label *label, *W_label, *E_label, *N_label, *S_label;
	Form *form;
	DialClass *dial;
	Label *azimuth_label, *incidence_label;
	Button *maximum_button, *origin_button;
	TextField *azimuth_text, *incidence_text, *maximum_text, *origin_text;
	TextField *distance_text;
	Choice *phase_choice;
	ScrollBar *scrollbar;

	vector<int> num_cmpts;
	gvector<Waveform *> wlist;
	bool *same_methods;
	Waveform *last_rotated[2];

	Iaspei *iaspei;
	int id, p_npts, s_npts;
	float p_tt[500], p_dist[500], p_ray_p[500];
	float s_tt[500], s_dist[500], s_ray_p[500];

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *action_event);
	void dialAction(DialCallbackStruct *s);
	void rotate(void);
	void rotateToMax(void);
	void rotateToMax(bool windowed, gvector<Waveform *> &wvec);
	void rotateToOrigin(void);
	void unrotate(void);
	void setAzimuth(void);
	void setIncidence(void);
	void saveAzInc(void);
	void scrollAction(XmScrollBarCallbackStruct *s);

	void mapAzimuth(Waveform *w, double az);
	void getRayParameters(double depth);
	void computeDistance(double incidence);
	void drawOnMap(const char *sta, double sta_lat, double sta_lon,
			double az, double distance);
};

} // namespace libgrot

#endif
