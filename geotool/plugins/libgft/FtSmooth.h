#ifndef _FT_SMOOTH_H
#define _FT_SMOOTH_H

#include "motif++/ParamDialog.h"
#include "motif++/MotifDecs.h"

namespace libgft {

class FtSmooth : public ParamDialog
{
    public:
	FtSmooth(const char *name, Component *parent);
	~FtSmooth(void);

	void setWidth(double width);
	double getWidth(void) { return width; }
	ParseCmd parseCmd(const string &cmd, string &msg);

    protected:

	Button		*close_button, *apply_button;
	Separator	*sep;
	RowColumn	*controls, *rc;
	Label		*label;
	TextField	*width_text;
	double		width;

	void createInterface(void);
	void actionPerformed(ActionEvent *a);

    private:
};

} // namespace libgft
#endif
