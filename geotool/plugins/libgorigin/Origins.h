#ifndef _ORIGINS_H
#define _ORIGINS_H

#include "motif++/Frame.h"
#include "DataReceiver.h"
#include "motif++/MotifDecs.h"
#include "gobject++/cvector.h"
#include "MultiTable.h"
#include "BasicMap.h"

/** @defgroup libgorigin plugin Origins
 */

class DataSource;
class CPlotClass;
class TtPlotClass;
class MultiTable;
class WaveformPlot;
class GVector;
class BasicMap;
class OridList;

namespace libgorigin {

class UndoDeleteOrigin;
class UndoEditOrigin;

/** Origins window.
 *  @ingroup libgorigin
 */
class Origins : public Frame, public DataReceiver
{
    friend class UndoDeleteOrigin;
    friend class UndoEditOrigin;

    public:
	Origins(const char *, Component *, DataSource *);
	~Origins(void);

	ParseCmd parseCmd(const string &cmd, string &msg);
	void parseHelp(const char *prefix);

    protected:
	WaveformPlot	*wp;
	FileDialog	*fileDialog;
	FileDialog	*saveDialog;
	BasicMap	*map;

	// File menu
	Button *close_button;

	// Edit menu
	Button *create_button, *delete_button, *edit_button;
	UndoButton *undo_button;

	// View menu
	Button *attributes_button;
	Toggle *time_toggle, *orid_toggle;
	Button *remove_button, *select_button;
	Menu *sort_menu;

	// Option menu
	Button *assoc_origin_button;

	// Help menu
	Button *help_button;

	OridList *orid_list;
	MultiTable *table;
	Separator *sep;

	void createInterface(void);
	void init(void);
	void actionPerformed(ActionEvent *action_event);
	void list(void);
	void create(void);
	void editSave(gvector<MultiTableEdit *> *v);
	void removeOrigin(void);
	void deleteOrigins(void);
	void selectRow(MmTableSelectCallbackStruct *c);
	void setButtonsSensitive(void);
	void selectAssociatedArrivals(void);
	void associateOrigin(void);
	void selectOriginFromMap(MapPlotSource *src);

	bool undoEditOrigin(UndoEditOrigin *);

    private:

};

} // namespace libgorigin

#endif
