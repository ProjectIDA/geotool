#ifndef _SHAPE_TABLE_H
#define _SHAPE_TABLE_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Map.h"

class Table;

namespace libgmap {

class MapThemeFile;

class ShapeTable : public FormDialog
{
    public:
	ShapeTable(const char *, Component *, MapThemeFile *, DBFHandle, int);
	~ShapeTable(void);

	Table *shapeTable() { return table; }
	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

	Button *turn_on_button, *turn_off_button;

    protected:
	MapThemeFile *tf;
	DBFHandle dbf;
	int ncols, nrows, irow;

	InfoArea *info_area;
	RowColumn *rc;
	Button *close_button, *apply_button;
	Table *table;

	void actionPerformed(ActionEvent *a);
	bool fillTable(void);
	void apply(void);
	void turnOn(void);
	void turnOff(void);
	void checkForChange(void);

	static Boolean workProc(XtPointer client);

    private:

};

} // namespace libgmap

#endif
