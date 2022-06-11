#ifndef _MAP_OVERLAY_H
#define _MAP_OVERLAY_H

#include "gobject++/Gobject.h"
#include "widget/Map.h"

namespace libgmap {

class MapWindow;

class MapOverlay : public Gobject
{
    public:
	MapOverlay(const string &name, MapWindow *mapWindow);
	~MapOverlay(void);

	MapWindow	*mw;
	Map		*map;
	int		path;
	int		name;
	int		num_ids;
	int		*id;
	bool		selected;
	bool		has_been_read;

	void readOverlayName(void);
	void readOverlay(int display);
	static bool isCommand(const char *buf);
	static int getSymType(const char *buf);
	static int getLine(FILE *fp, char *line, int len, int *line_no);

    protected:
	void getOverlayDefaults(char * buf, const char *overlay_name,
			SymbolInfo *default_sym, Pixel fg);
	void getLineInfo(char *buf, Pixel bg, Pixel fg, LineInfo *line);

    private:
	static int getDashLen(char *);
	static void getDashes(char *, char *);
};

} // namespace libgmap

#endif /* _OVERLAY_H */
