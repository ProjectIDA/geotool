#ifndef _WORKING_H
#define _WORKING_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"


/**
 *  @ingroup libgx
 */
class Working : public FormDialog
{
    public:
	Working(const string &name, Component *parent, int num,
		const char **labels);
	Working(const string &name, Component *parent, int num,
		const char **labels, ActionListener *listener);
	Working(const string &name, Component *parent, const string &title,
		const string &update_msg);
	Working(const string &name, Component *parent, const string &title,
		const string &update_msg, ActionListener *listener);
	~Working(void);

	void actionPerformed(ActionEvent *action_event);

	bool update(const string &name, int num_records);
	bool update(int num_records);

    protected:

	int	num_labels;
	Label	*label;
	Button	*stop;
	Separator *sep;
	RowColumn *rc;
	Label	**label_comps;
	Label	**result_comps;
	string	update_msg;

	void createInterface(const string &title, int num, const char **labels);

    private:
	bool	working;
};

#endif
