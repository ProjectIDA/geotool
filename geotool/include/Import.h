#ifndef _IMPORT_H
#define _IMPORT_H

#include <vector>
#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"
#include "gobject++/ghashtable.h"
#include "TableViewer.h"

class FileDialog;

/**
 *  @ingroup libgx
 */
class Import : public FormDialog
{
    public:
	Import(const string &, Component *, TableViewer *);
	~Import(void);

	void actionPerformed(ActionEvent *action_event);
	ParseCmd parseCmd(const string &cmd, string &msg);
	static void parseHelp(const char *prefix);
	bool import(FILE *fp, const string &tables, const string &members,
			const string &assignments);
	bool importStdin(const string &tables, const string &members,
			const string &assignments);
	ParseCmd parseImport(const string &c, string &msg);

    protected:
	TableViewer *tv;
	FileDialog *open_file;
	ghashtable<long> ids;

	RowColumn *controls, *rc, *rc2;
	Button *close_button, *import_button;
	Separator *sep;
	Label *label, *tables_label, *members_label, *assignments_label;
	Button *prefix_button;
	ScrolledWindow *sw;
	TextField *file_text, *tables_text, *members_text, *assignments_text;

	void chooseFile(void);
	bool import(void);
	void clearNames(void);
	bool startImport(const string &tables, const string &members);
	bool importLine(char *line, const string &assignments);
};

#endif
