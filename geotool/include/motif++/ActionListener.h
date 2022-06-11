#ifndef _ACTION_LISTENER_H
#define _ACTION_LISTENER_H

class ActionEvent;

/** Abstract base class for Listeners
 *  @ingroup libmotif
 */
class ActionListener
{
    public:
	virtual ~ActionListener() {}

	/** The action (event) callback function. A ActionListener can "listen"
	 * for action events from Component by registering itself as a listener
	 * with the Component.  When an action occurs on a Component, that
	 * Component calls the actionPerformed function belonging to all of the
	 * ActionListener instances that have registered as listeners
	 * for the particular type of action. The ActionEvent class contains
	 * the action command string, the source Component of the action, the
	 * reason for the action, and the callback data associated with the
	 * action. Each derived class must implement the actionPerformed()
	 * function.
	 * @param[in] action_event a pointer to an ActionEvent object.
         * @sa addActionListener
	 */
	virtual void actionPerformed(ActionEvent *action_event) = 0;

    protected:
	ActionListener() {
	}
};

#endif
