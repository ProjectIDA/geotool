#ifndef _COLOR_TABLE_H
#define _COLOR_TABLE_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "widget/Table.h"

class ColorTable : public FormDialog
{
    public:
	ColorTable(const string &name, Component *parent);
	~ColorTable(void);

	void actionPerformed(ActionEvent *a);
	ParseCmd parseCmd(const string &cmd, string &msg);

	int numColors() { return num_colors; }
	Pixel pixel(int i) { return pixels[i]; }
	bool setColors(const string &names, string &msg);

	static const char * rgbToName(short r, short g, short b);

    protected:
	RowColumn	*rc;
	Button		*close_button, *apply_button, *default_button;
	Separator	*sep;
	Table		*table;

	Form		*form;
	Button		*color_plus_button, *color_minus_button;

	int	num_colors, num_tmp_colors;
	Pixel	*pixels, *tmp_pixels;

	void loadTable(void);
	void editColor(MmTableEditCallbackStruct *e);
	void saveColors(void);
	void setButtonsSensitive(void);
	void addRow(void);

    private:
};

#endif
