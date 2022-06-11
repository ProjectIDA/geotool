#ifndef _WAVEFORMS_H
#define _WAVEFORMS_H

#include "motif++/Frame.h"
#include "DataReceiver.h"
#include "motif++/MotifDecs.h"
#include "gobject++/DataSource.h"
#include "CSSTable.h"

class Waveform;
class WaveformTable;
class SegmentTable;

/**
 *  @ingroup libgx
 */
class Waveforms : public Frame, public DataReceiver
{
    friend class WaveformTable;

    public:
	Waveforms(const string &name, Component *parent, DataSource *ds);
	~Waveforms(void);

	void actionPerformed(ActionEvent *action_event);

    protected:

	// File Menu
	Button	*close_button;

	// Edit Menu
	Button	*remove_button;

	// View Menu
	Button	*attributes_button, *deselect_all_button, *promote_rows_button;
	Button	*promote_cols_button, *sort_button, *sort_selected_button;
	Button	*sort_unique_button, *expand_button, *reverse_button;
	Button	*show_all_button, *apply_sort_button;
	Button	*segments_button;

	// Help Menu
	Button	*help_button;

	CSSTable *table;
	SegmentTable *segment_table;

	gvector<Waveform *> ws;

	void createInterface(void);
	void list(void);
	void setButtonsSensitive(void);
	void applySort(void);
	void selectWaveform(void);
	void selectRow(MmTableSelectCallbackStruct *c);

    private:
};

#endif
