#ifndef _FILE_CHOICE_H
#define _FILE_CHOICE_H

#include "motif++/Choice.h"

/** A subclass of Choice for the selection of files or directories.
 *  The last item in the Choice menu is either "(choose file)" or "(choose
 *  directory)". When this item is selected, a popup FileDialog allows the
 *  selection of a file or directory that is not already in the FileChoice menu.
 *  The following code illustrates the usage of the FileChoice class.
 *  \include motif++/FileChoiceExample.cpp
 *  @ingroup libmotif
 */
class FileChoice : public Choice
{
    public:

	/** Constructor */
	FileChoice(const string &name, Component *parent, Arg *args=NULL,
		int n=0, ActionListener *listener=NULL, bool choose_file=true);

	/** Destructor */
	~FileChoice(void);

	/** Add a choice */
	void addItem(const string &);
	/** Add a Separator */
	void addSeparator(void);
	/** Remove all choices */
	void removeAllChoices(void);

    protected:

	static void chooseFileCB(Widget, XtPointer, XtPointer);
	static void chooseDirCB(Widget, XtPointer, XtPointer);
	void addChooseFile(bool);
	void chooseFile(bool);
};

#endif
