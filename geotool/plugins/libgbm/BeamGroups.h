#ifndef _BEAM_GROUPS_H
#define _BEAM_GROUPS_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"
#include "Beam.h"

namespace libgbm {

class BGStations;
class AddGroup;

class BeamGroups : public Frame
{
    public:
	BeamGroups(const char *, Component *);
	~BeamGroups(void);

    protected:
	// File menu
	Button *close_button;

	// Edit menu
	Button *add_button, *delete_button;

	// View menu
	Button *attributes_button;

	// Option menu
	Button *stations_button;

	// Help menu
	Button *group_help_button;

	Table *table;

	string recipe_dir, recipe_dir2;
	vector<BeamGroup> groups;
	BGStations *bg_stations;
	AddGroup *add_group;

	void init(void);
	void createInterface(void);
	void actionPerformed(ActionEvent *a);
	void list(void);
	void readGroups(void);
	void selectGroup(MmTableSelectCallbackStruct *c);
	void setButtonsSensitive(bool set);
	void listStations(void);
	void deleteGroup(void);

    private:

};

} // libgbm

#endif
