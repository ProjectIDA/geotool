#ifndef _ARRIVALS_H
#define _ARRIVALS_H

#include "motif++/Frame.h"
#include "motif++/MotifDecs.h"
#include "DataReceiver.h"
#include "gobject++/gvector.h"
#include "widget/Table.h"
#include "gobject++/CssTables.h"

/** @defgroup libgarrival plugin Arrivals
 */

class DataSource;
class DataReceiver;
class WaveformPlot;
class MultiTable;
class MultiTableEdit;
class Waveform;
class OridList;

namespace libgarrival {

class AmpMag;
class MeasureAmpPer;
class Stassocs;
class UndoDeleteArrival;
class UndoEditArrival;

/** Arrivals plugin window.
 *  @ingroup libgarrival
 */
class Arrivals : public Frame, public DataReceiver
{
    friend class UndoDeleteArrival;
    friend class UndoEditArrival;

    public:
	Arrivals(const string &, Component *, DataSource *);
	~Arrivals(void);

	void setDataSource(DataSource *ds);
	void setVisible(bool visible);
	ParseCmd parseCmd(const string &cmd, string &msg);
	ParseVar parseVar(const string &name, string &value);
	void parseHelp(const char *prefix);

    protected:
	WaveformPlot	*wp;
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	AmpMag		*amp_mag_window;
	MeasureAmpPer	*measure_window;
	Stassocs	*stassocs_window;

	// File menu
	Button *open_button, *close_button;

	// Edit menu
	Button *add_sta_button, *add_net_button, *delete_button;
	Button *edit_button, *rename_button;
	UndoButton *undo_button;

	// View menu
	Menu *align_menu;
	Button *align_button1, *align_button2, *align_button3, *align_button4;
	Button *align_button5, *align_button6, *attributes_button;
	Button *pick_attributes_button, *loc_attributes_button;
	Toggle *selected_orid_toggle;
	Toggle *main_only_toggle;
	Button *remove_button, *select_all_button, *deselect_all_button;
	Menu *select_menu, *sort_menu;
	Toggle *time_toggle, *sta_time_toggle, *selected_toggle;
	Toggle *orid_time_toggle, *orid_sta_time_toggle;
	Toggle *delta_sta_time_toggle, *phase_delta_time_toggle;
	Button *sel_cols_button, *zoom_sp_button, *zoom_lp_button;

	// Option menu
	Button *amp_button;
	Toggle *auto_amp_toggle, *single_toggle;
	Button *assoc_origin_button, *disassoc_origin_button;
	Button *assoc_stassoc_button, *disassoc_stassoc_button;
	Button *measure_button, *stassocs_button;

	// Help menu
	Button *help_button;

	Form *form;
	Choice *list_choice;
	ScrolledWindow *sw;
	List *alist;
	TextField *add_text;
	Separator *sepv;
	MultiTable *table;
	OridList *orid_list;

	enum SortType {
	    TIME,
	    STA_TIME,
	    SELECTED,
	    ORID_TIME,
	    ORID_STA_TIME,
	    DELTA_STA_TIME,
	    PHASE_DELTA_TIME,
	    SELECTED_COLUMNS,
	    PROMOTE_SELECTED,
	    SORT_ON_COLUMNS
	};

	SortType listing_sort;

	string time_format;
	int time_column;

	string sort_on_columns;
	bool warn_if_hidden;
	bool phase_line_added;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *action_event);
	void list(void);
	void arrivalSelect(void);
	void selectRow(MmTableSelectCallbackStruct *c);
	void phaseLine(void);
	void addArrival(bool add_to_net);
	void excludeDuplicates(gvector<Waveform *> &w, double x,
		vector<bool> &do_it);

	void arrivalZoom(bool sp);
	void alignWaveforms(const string &cmd);
	void alignPredicted(void);
	void alignMinMax(bool align_min);
	void selectAll(bool select);
	void tableSorts(void);
	void updateArrivals(void);
	void associateWithOrigin(bool associate);
	void associateWithStassoc(bool associate);
	void setButtonsSensitive(void);
	void phaseLineButtons(bool set);
	void selectColumn(void);
	void sortByColumns(void);
	void renameArrival(const string &phase);
	void deleteArrivals(void);
	void editOn(void);
	void editCancel(void);
	void editSave(gvector<MultiTableEdit *> *);
	void renamePhaseLine(void);
	void retime(CssArrivalClass *a);

	bool undoEditArrival(UndoEditArrival *);


//	void openDatabase(void);

    private:

};

} // namespace libgarrival

#endif
