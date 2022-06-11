#ifndef _PARAMDIALOG_H
#define _PARAMDIALOG_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"


/** A subclass of FormDialog with parameter input functions.
 *  In the example code below, the Example subclass of ParamDialog creates two
 *  TextField children. It enables the XmNactivateCallback callback type, so
 *  other components can receive callbacks from it. When the Apply button is
 *  activated, the Example.actionPerformed routine notifies all listeners
 *  via the doCallbacks function. The GetParam.actionPerformed function then
 *  calls the ParamDialog.getInt and ParamDialog.getDouble functions.
 *  \include ParamDialogExample.cpp
 *  @ingroup libmotif
 */
class ParamDialog : public FormDialog
{
    protected:
	// can only be constructed by a subclass
	ParamDialog(const string &name, Component *parent);

    public:
	~ParamDialog(void);

	bool getDouble(const string &name, double *d);
	bool getInt(const string &name, int *n);
	bool getBool(const string &name, bool *b);
	bool getString(const string &name, char **s);
	bool getString(const string &name, char *s, int len);
	bool setString(const string &name, const string &s,
			bool do_callback=false);
	TextField *getTextField(const string &name);
	Toggle *getToggle(const string &name);
	bool setLabel(const string &name, const string &s);

    private:
};

#endif
