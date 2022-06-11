#ifndef _TEXT_DIALOG_H_
#define _TEXT_DIALOG_H_

#include <string>
#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "motif++/TextField.h"

class TextDialog : public FormDialog
{
    public:
	TextDialog(const string &, Component *, int max_num_lines,
		const string &text="");
	TextDialog(const string &, Component *, const string &text="");
	~TextDialog(void);

	void setText(const string &text);
	void append(const string &text);
	void save(void);
	void setMaxLines(int num) { text_field->setMaxLines(num); }
	int getMaxLines(void) { return text_field->maxLines(); }
	TextField *textField(void) { return text_field; }

    protected:
	RowColumn *controls;
	Button *close_button, *save_button;
	ScrolledWindow *sw;
	TextField *text_field;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:

};

#endif
