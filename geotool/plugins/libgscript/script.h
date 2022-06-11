#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "config.h"
#include "motif++/MotifClasses.h"

namespace libgscript {

/** The Script PlugIn subclass.
 *  @ingroup libgscript
 */
class gscriptPlugin : public PlugIn
{
    public :
	double a[20], b;
	float c;
	int n;

	gscriptPlugin(TopWindow *tw_parent, DataSource *ds, DataReceiver *dr)
			: PlugIn(tw_parent, ds, dr)
	{
	    // initialize some example variables
	    memset(a, 0, sizeof(a));
	    b = 3.67;
	    c = 89.1;
	    n = 45;
	}
	~gscriptPlugin(void) { }
	void actionPerformed(ActionEvent *action_event) {}

	virtual ParseCmd parseCmd(const string &cmd, string &msg);
	virtual ParseVar parseVar(const string &name, string &value);

	ParseCmd parseCmd1(const string &cmd, string &msg);
	ParseCmd parseCmd2(const string &cmd, string &msg);
};

} // namespace libgscript

#endif
