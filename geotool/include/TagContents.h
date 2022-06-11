#ifndef _TAG_CONTENTS_H
#define _TAG_CONTENTS_H

#include <string>
#include "SelectOrder.h"
#include "motif++/Button.h"
#include "motif++/TextField.h"

/**
 *  @ingroup libgx
 */
class TagContents : public SelectOrder
{
    public:
	TagContents(const string &name, Component *parent, int num,
		const char **list, int num_selected, int *selected,
		string user_string);
	~TagContents(void);

	void actionPerformed(ActionEvent *action_event);

	char *getText() { return user_text->getString(); }
	bool selectedOnly() { return selected_only->state(); }
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);

    protected:

	TextField	*user_text;
	Toggle		*selected_only;

    private:
};

#endif
