#ifndef _FT_WINDOWS_H
#define _FT_WINDOWS_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgft {

typedef struct
{
    int num_windows;
    int overlap;
} FtWindowsStruct;

class FtWindows : public ParamDialog
{
    public:
	FtWindows(const char *name, Component *parent);
	~FtWindows(void);

	ParseCmd parseCmd(const string &cmd, string &msg);

    protected:

	Button		*close_button, *apply_button;
	Separator	*sep;
	RowColumn	*controls, *rc, *rc1, *rc2;
	TextField	*num_windows_text, *overlap_text;
	Scale		*num_windows_scale, *overlap_scale;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:
};

} // libgft

#endif
