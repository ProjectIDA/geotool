#ifndef ACTION_EVENT_H
#define ACTION_EVENT_H

#include "gobject++/Gobject.h"
using namespace std;

class Component;

/** Event information for action callbacks.
 *  This class is used to communicate action event information to Component
 *  listeners via the actionPerformed function. If a Component needs to listen
 *  for action events that occur on another Component, it first calls the other
 *  component's addActionListener function to register itself as a listener for
 *  a particular type of action. Each time this action occurs, the listener
 *  Component's actionPerformed function will be called with an ActionEvent
 *  object as it's argument.
 *  @ingroup libmotif
 */
class ActionEvent : public Gobject
{
    public:

	/** A constructor.
	 *  @param[in] action_src the Component action source.
	 *  @param[in] action_widget the Widget action source.
	 *  @param[in] action_command the string ID of the source. It defaults
	 *	to the name of the source Component.
	 *  @param[in] callback_data the callback data. (can be NULL)
	 *  @param action_reason this is the action_type string.
	 */
	ActionEvent(Component *action_src, Widget action_widget,
		const string &action_command, XtPointer callback_data,
		const string &action_reason="") : source(action_src),
		widget(action_widget), command(action_command),
		reason(action_reason), calldata(callback_data)
	{ }

	ActionEvent(const ActionEvent &a) :
	    source(a.source), widget(a.widget), command(a.command),
	    reason(a.reason), calldata(a.calldata)
	{ }

	ActionEvent & operator=(const ActionEvent &a) {
	    source = a.source;
	    widget = a.widget;
	    command = a.command;
	    reason = a.reason;
	    calldata = a.calldata;
	    return *this;
	}

	/** The destructor. Free string space. */
	~ActionEvent(void) { }

	/** Get the Component source. */
	Component *getSource(void) { return source; }
	/** Get the reason. */
	const char *getReason(void) { return reason.c_str(); }
	/** Get the reason. */
	void getReason(string &s) { s.assign(reason); }
	/** Get the Widget source. */
	Widget getWidget(void) { return widget; }
	/** Get the action command string. */
	const char *getActionCommand(void) { return command.c_str(); }
	/** Get the action command string. */
	void getActionCommand(string &s) { s.assign(command); }
	/** Change the action command string */
	void setCommandString(const string &cmd) {
	    command = cmd;
	}
	/** Get the action callback data. */
	XtPointer getCalldata(void) { return calldata; }

    protected:
	Component	*source; //!< The Component source of the action.
	Widget		widget;  //!< The Widget source of the action.
	string		command; //!< The action command string.
	string		reason;  //!< The action reason.
	XtPointer	calldata;//!< The action callback data.
};

#endif
