#ifndef _TABLEATTRIBUTES_H
#define _TABLEATTRIBUTES_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/CssTableClass.h"

typedef struct
{
    char	*name;
    char	format[20];
    int		table;
    int		member;
    int		order;
} TAttribute;


class Table;


/**
 *  @ingroup libwgets
 */
class TableAttributes : public FormDialog
{
    public:
	TableAttributes(const string &name, Component *parent,
			const string &attribute_list);
	TableAttributes(const string &name, Component *parent,
			const string &attribute_list,
			const string &display_list);
	TableAttributes(const string &name, Component *parent, int num_members,
			CssClassDescription *des);
	TableAttributes(const string &name, Component *parent, int num_members,
			CssClassDescription *des, const string &display_list);
	TableAttributes(const string &name, Component *parent, int num_members,
			CssClassDescription *des, int num_extra, const char **extra,
			const char **extra_format, const string &display_list);
	TableAttributes(const string &name, Component *parent, int num_tables,
			int *num_members, CssClassDescription **des, int num_aux,
			int num_aux_members, CssClassDescription *aux_des,
			int num_extra, const char **extra,
			const char **extra_format, const string &display_list);

	~TableAttributes(void);

	void actionPerformed(ActionEvent *action_event);

	int displayAttributes(TAttribute ***a) { 
	    *a = attributes;
	    return num_display_attributes;
	}
        void setOrder(vector<int> &order);
	TAttribute getAttribute(int i);
	TAttribute getAttribute(const string &name);
	void setDisplayAttributes(const string &display_list);
	int numAttributes(void) { return num_attributes; }
	int numDisplayAttributes(void) { return num_display_attributes; }
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:

	int 	num_attributes;
	int	num_display_attributes;
        TAttribute **orig_attributes;
        TAttribute **attributes;
        TAttribute **test_attributes;
	Pixel	*colors;
	Button	*close_button;
	Button	*apply_button;
	Button	*reset_button;
	Button	*help_button;
	Button	*select_all_button;
	Button	*deselect_all_button;
	Table	*table;
	Separator *sep;
	RowColumn *controls;

	void createInterface(void);
	void init(const string &);
	void init(int num_members, CssClassDescription *des);
	void init(int num_tables, int *num_members, CssClassDescription **des,
		int num_aux, int num_aux_members, CssClassDescription *aux_des);
	void getExtra(int num_extra, const char **extra,
			const char **extra_format);
	void createAttributes(const string &);
	void list(void);
	bool changed(void);
	void apply(void);

    private:
};

#endif
