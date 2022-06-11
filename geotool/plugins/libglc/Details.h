#ifndef _DETAILS_H
#define _DETAILS_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

namespace libglc {

class Details : public FormDialog
{
    public:
	Details(const char *, Component *, char *text);
	~Details(void);

	void setText(char *text);
	void save(void);

    protected:
	RowColumn *controls;
	Button *close_button, *save_button;
	ScrolledWindow *sw;
	TextField *text_field;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

} // namespace libglc

#endif
