#ifndef _STASSOCS_H
#define _STASSOCS_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "CSSTable.h"

class DataSource;

namespace libgarrival {

class Stassocs : public Frame, public DataReceiver
{
    public:
	Stassocs(const char *, Component *, DataSource *);
	~Stassocs(void);

    protected:
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;

	// File menu
	Button *open_button, *close_button;

	// Edit menu
	Button *delete_button, *edit_button;

	// View menu
	Button *attributes_button;

	// Help menu
	Button *help_button;

	ctable<CssStassocClass> *table;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *a);
	void list(void);

    private:
};

} // namespace libgarrival

#endif
