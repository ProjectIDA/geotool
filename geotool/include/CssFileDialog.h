#ifndef _CSS_FILE_DIALOG_H_
#define _CSS_FILE_DIALOG_H_

#include "motif++/FileDialog.h"
#include "motif++/MotifDecs.h"

class WaveformWindow;
class TableQuery;
/**
 *  @ingroup libgx
 */
class CssFileDialog : public FileDialog
{
    public:
	CssFileDialog(const string &name, Component *parent, FileType file_type,
			const string &directory, const string &table);
	CssFileDialog(const string &name, Component *parent, FileType file_type,
			const string &directory, const string &table,
			const string &open_button_name);
	CssFileDialog(const string &name, WaveformWindow *parent,
			FileType file_type, const string &directory,
			const string &table);
	CssFileDialog(const string &name, WaveformWindow *parent,
			FileType file_type, const string &directory,
			const string &table, const string &open_button_name);
	~CssFileDialog(void) {}

	void actionPerformed(ActionEvent *action_event);

	CssFileDialog *getCssFileDialogInstance(void) { return this; }

    protected:
	virtual void getFileSuffixes(const string &table);
	void addListButton(DataReceiver *data_receiver);

	TableQuery *tq;
	DataReceiver *dr;
	Button *list_button;
};
#endif
