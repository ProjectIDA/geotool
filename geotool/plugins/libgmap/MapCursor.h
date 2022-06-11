#ifndef _MAP_CURSOR_H_
#define _MAP_CURSOR_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

class Map;
class WaveformWindow;
class DataSource;

namespace libgmap {

class MapWindow;

class MapCursor : public FormDialog
{
    public:
	MapCursor(const char *name, Component *parent, MapWindow *mapWindow,
		DataSource *ds);
	~MapCursor(void);

	void setVisible(bool visible);

    protected:
	MapWindow	*mw;
	WaveformWindow	*ww;
	Map		*map;
	bool		added_crosshair;

	RowColumn	*controls, *rc, *rc1, *rc2, *rc3;
	Button		*close_button, *sort_button, *help_button;
	Separator	*sep;

	Label		*label, *region_label;
	TextField	*latitude_text, *longitude_text, *depth_text;
	Scale		*latitude_scale, *longitude_scale, *depth_scale;

	void actionPerformed(ActionEvent *a);
	void setScale(Scale *scale, double value);
	void positionCrosshair(void);
	void crosshairCB(void);
	void seismicRegion(double lat, double lon);

    private:
};

} // namespace libgmap

#endif
