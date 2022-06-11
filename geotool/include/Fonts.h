#ifndef _FONTS_H_
#define _FONTS_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"


/**
 *  @ingroup libgx
 */
class Fonts : public FormDialog
{
    public:
	Fonts(const string &, Component *);
	~Fonts(void);

	void actionPerformed(ActionEvent *action_event);

	static void showFonts(void);
	void setVisible(bool visible);
	static ParseCmd parse(const string &cmd, string &msg);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:
	Label *label;
	RowColumn *rc, *controls;
	Button *apply_button, *close_button;
	Separator *sep;

	Label *lab1, *lab2, *lab3, *lab4, *lab5, *lab6, *lab7, *lab8;
	Choice *type1, *type2, *type3, *type4, *type5, *type6, *type7, *type8;
	Choice *size1, *size2, *size3, *size4, *size5, *size6, *size7, *size8;

	void fontsInit(void);
	void setFontMenus(void);
	void setFonts(void);
};

#endif
