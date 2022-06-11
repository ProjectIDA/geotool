#ifndef _RECIPES_H
#define _RECIPES_H

#include <vector>
#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "widget/Table.h"
#include "Beam.h"
#include "TravelTime.h"
#include "gobject++/CssTables.h"
using namespace std;

class DataSource;
class CPlotClass;
class Waveform;
class Iaspei;
class IIRFilter;
class WaveformPlot;
class WaveformWindow;
class GParseSource;
class RefSta;

namespace libgbm {

/** @defgroup libgbm plugin Origin Beams and Detection Beams
 */

enum RecipeType {
    ORIGIN_RECIPES,
    DETECTION_RECIPES
};

class BeamGroups;
class AddRecipe;

/** Recipes window (origin or detection).
 *  @ingroup libgbm
 */
class Recipes : public Frame, public DataReceiver
{
    public:
	Recipes(const char *, Component *, RecipeType type, DataSource *);
	~Recipes(void);

	void setDataSource(DataSource *ds);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	RecipeType	type;

	// File menu
	Button *close_button;

	// Edit menu
	Button *add_button, *delete_button, *edit_button;

	// View menu
	Button *attributes_button, *deselect_all_button;
	Toggle *beam_window_toggle, *replace_beam_toggle;

	// Option menu
	Button *beam_button, *beam_selected_button, *groups_button;
	Menu *location_menu;
	Toggle *dnorth_toggle, *reference_toggle, *array_center_toggle;
	Button *slowness_button, *ref_stations_button;

	// Help menu
	Button *beam_help_button;

	Table *table;

	vector<BeamRecipe> *recipes;
	TravelTime *travel_time;
 	WaveformWindow *beam_window;
	BeamGroups *beam_groups;
	RefSta *ref_sta_window;
	WaveformPlot *wp;
	Beam *gbeam;
	AddRecipe *add_recipe_window;

	void init(void);
	void createInterface(void);
	void actionPerformed(ActionEvent *action_event);
	void list(void);
	void setButtonsSensitive(void);
	void readRecipes(bool force_read);
	void selectRecipe(MmTableSelectCallbackStruct *c);
	void beamWaveforms(bool selected_only);
	void makeBeam(CssOriginClass *origin, BeamRecipe &recipe,
		bool selected_only);
	int getBeamStations(BeamRecipe &recipe,
		gvector<Waveform *> &all_ws, CssOriginClass *origin,
		gvector<Waveform *> &ws, vector<double> &weights);
	void loadRefSta(void);
	void saveRecipe(void);
	void deleteRecipe(void);


    private:

};

} // libgbm

#endif
