#ifndef _QUERY_VIEWS_H
#define _QUERY_VIEWS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

#define MAX_VIEWS 10

class QueryViews : public FormDialog
{
    public:
	QueryViews(const string &name, Component *parent, Menu *views_menu);
	~QueryViews(void);

	typedef struct {
	    char	*name;
	    char	*query;
	    Button	*b;
	} ViewStruct;

	int		num_views;
	ViewStruct	views[MAX_VIEWS];

    protected:
	Menu *views_menu;
	Button *edit_views;

	RowColumn *controls;
	Button *apply_button, *cancel_button, *default_button;
	Separator *sep;
	ScrolledWindow *sw;
	Form *work;
	TextField *query_label[MAX_VIEWS];
	ScrolledWindow *query_sw[MAX_VIEWS];
	TextField *query_text[MAX_VIEWS];

	void actionPerformed(ActionEvent *a);
	void init(void);
	void setDefaultViews(void);
	void apply(void);
	void cancel(void);
	void saveViews(void);

    private:

};

#endif
