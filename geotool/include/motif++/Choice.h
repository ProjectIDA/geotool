#ifndef _CHOICE_H
#define _CHOICE_H

#include "motif++/Component.h"

/** A class for the XmOptionMenu widget.
 *  @ingroup libmotif
 */
class Choice : public Component
{
    public:

	/** Constructor */
	Choice(const string &name, Component *parent,
		ActionListener *listener=NULL, Arg *args=NULL, int n=0);
	/** Constructor */
	Choice(const string &name, Component *parent, Arg *args, int n);
	/** Destructor */
	~Choice(void);

	/** Add an item */
	void addItem(const string &);

	/** Add a Separator */
	void addSeparator(void);

	/** Get the PullDown widget */
	Widget pullDown(void) { return pulldown; }

	/** Get the choice */
	const char *getChoice(void);

	/** Get the choice */
	void getChoice(string &choice) { choice.assign(getChoice()); }

	/** Set the choice */
	bool setChoice(const string &name, bool do_callbacks=false,
		bool ignore_case=false);

	/** Remove all choices */
	void removeAllChoices(void);

	/** Parse string cmd */
	ParseCmd parseCmd(const string &cmd, string &msg);

	ParseVar parseVar(const string &name, string &value);

	/** print command strings */
	void parseHelp(const char *prefix);

        virtual void destroy(void);

    protected:

	Widget	pulldown; //!< the PulldownMenu widget
	bool	first_item;

	void init(Component *parent, Arg *args, int n);
	static void activateCallback(Widget, XtPointer, XtPointer);

    private:
};

#endif
