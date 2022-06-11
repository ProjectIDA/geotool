#ifndef _MAP_OVERLAY_FORM_H
#define _MAP_OVERLAY_FORM_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/gvector.h"

class Map;

namespace libgmap {

class MapWindow;
class MapOverlay;

class MapOverlayForm : public FormDialog
{
    public:
	MapOverlayForm(const char *name, Component *parent,
			MapWindow *mapWindow);
	~MapOverlayForm(void);

	void listOverlay(MapOverlay *o);
	void displayOverlay(MapOverlay *o);
	void addOverlayName(MapOverlay *o);
	bool isOverlayId(int id);
	List *getList(void) { return list; }
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	MapWindow	*mw;
	Map		*map;
	Label		*label;
	Button		*close_button, *remove_button, *help_button;
	Separator	*sep;
	RowColumn	*controls;
	ScrolledWindow	*sw;
	List		*list;
	gvector<MapOverlay *> *overlays;

	void actionPerformed(ActionEvent *a);
	void readOverlayDir(void);
	void readOverlayDir(const string &overlay_dir, bool warn);
	void overlaySelect(void);
	void overlayRemove(void);
	void readOverlay(MapOverlay *o, int display);
	void scrollRight(void);
	bool duplicatePath(MapOverlay *o_new);

    private:
};

} // namespace libgmap

#endif
