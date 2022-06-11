#ifndef _PRINT_OPTIONS_H_
#define _PRINT_OPTIONS_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "PrintParam.h"
#include "widget/Axes.h"

/** 
 *  @ingroup libwgets
 */
class PrintOptions : public FormDialog
{
    public:
	PrintOptions(const char *, Component *);
	~PrintOptions(void);

	void actionPerformed(ActionEvent *action_event);

	void fillPrintStruct(PrintParam *p);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:
	Label *fonts;
	RowColumn *rc, *controls, *rc2, *rc_bot;
	Button *close_button, *help_button;
	Separator *sep1, *sep2;
	TextField *fontText[5], *sizeText[5], *typeText[5];

	RadioBox *ink_rb, *arrival_rb, *scale_rb, *tag_rb;
	Toggle *rb_bw, *rb_color, *rb_normal, *rb_close;
	Toggle *rb_full, *rb_half, *rb_standard, *rb_left;
	Choice *axes_font, *tick_font, *tag_font, *arrival_font, *main_font;
	Choice *font_menu[5], *size_menu[5], *type_menu[5], *line_width;

	static void addFontStyle(char *font, char *fontStyle);
};

#endif
