#ifndef _OPEN_FILE_H
#define _OPEN_FILE_H

#include "CssFileDialog.h"
#include "DataReceiver.h"
#include "motif++/MotifDecs.h"

class WaveformWindow;
class WaveformPlot;
class TableSource;
class SeedSource;
class Settings;
class ImportSeedStation;

/**
 *  @ingroup libgx
 */
class OpenFile : public CssFileDialog, public DataReceiver
{
    public:
	OpenFile(const string &, WaveformWindow *, FileType, const string &,
		const string &);
	OpenFile(const string &, WaveformWindow *, FileType, const string &,
		const string &, const string &);
	~OpenFile(void);

	void actionPerformed(ActionEvent *action_event);
	void showPreviewArea(void);
	bool previewFile(const string &file);

	WaveformPlot	*wplot;
	string		preview_file;

    protected:
	WaveformWindow	*parent_wplot;
	TableSource	*ds;
	SeedSource	*seed_source;
	Form		*form;
	Label		*date_label;
	Button		*tq_display_seed;
	Button		*tq_list_records;
	Button		*tq_list_blockettes;
	Button		*tq_blockette_contents;
	Button		*settings_button;
	Button		*import_button;
	Button		*import_seed_button;
	TextDialog	*blockette_window;
	Settings	*settings;
	ImportSeedStation	*import_seed_station;
	const char **table_names;
	int num_table_names;

	void init(const string &pattern);
	bool tableSuffix(const string &file);
	void showBlockettes();
	void listBlockettes();

    private:
};

#endif
