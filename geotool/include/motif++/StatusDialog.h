#ifndef _STATUS_DIALOG_H
#define _STATUS_DIALOG_H

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

/** A C-language interface to the StatusDialog class.
 * If this function is called with a status of 0,
 * a new StatusDialog is created and displayed. The function setParent must
 * be called before this function is called with a status equal to 0. When this
 * function is called with a status of 1, the StatusDialog scale value
 * indicator is repositioned to the value of the count argument.
 * @param[in] w this argument is ignored
 * @param[in] count if status is 0, this is the total number of computations.
 *	if status is 1, this is the current count. if status is 2, the window is
 *	closed.
 * @param[in] label_format a label containing an integer format (like %d)
 * @returns 0 or 1. 0 means stop the computation. 1 means continue the
 *	computation.
 */
int WorkingDialog(void *w, int count, int status, const char *label_format);

/** A dialog window for reporting the status of an operation. A message
 *  is created using snprintf with the label_format and total as arguments:
 *  \code
 *  snprintf(buf, sizeof(buf), label_format, total);
 *  \endcode
 *  For example, when many 224 FKs are computed, the StatusDialog is created
 *  with 
 *  \code
 *  new StatusDialog("Working", this, "Computing %d FKs", 224);
 *  \endcode
 *  A slider is displayed below the message "Computing 224 FKs". After each FK
 *  computation the update(int count) function is called, where count steps
 *  from 1 to 224. The sider value is set to count each time update is called.
 *  A screen image of the StatusDialog for this example is shown below.
 *  \image html StatusDialog.gif "The StatusDialog"
 *  \image latex StatusDialog.eps "The StatusDialog" width=3in
 *
 * There is a C-language interface to this class.
 * \code
extern "C" int WorkingDialog(void *w, int count, int status, const string &label_format)
 * \endcode
 * If this function is called with a status of 0,
 * a new StatusDialog is created and displayed. The function setParent must
 * be called before this function is called with a status equal to 0. When this
 * function is called with a status of 1, the StatusDialog scale value
 * indicator is repositioned to the value of the count argument.
 * @param[in] w this argument is ignored
 * @param[in] count if status is 0, this is the total number of computations.
 *	if status is 1, this is the current count. if status is 2, the window is
 *	closed.
 * @param[in] status 0, 1 or 2.
 * @param[in] label_format a label containing an integer format (like %d)
 * @returns 0 or 1. 0 means stop the computation. 1 means continue the
 *	computation.
 *  @ingroup libmotif
 */
class StatusDialog : public FormDialog
{
    public:
	StatusDialog(const string &name, Component *parent,
		const string &label_format, int total,
		ActionListener *listener=NULL);
	~StatusDialog(void);

	bool update(int count);
	void actionPerformed(ActionEvent *action_event);

	static void setParent(Component *parent);

    protected:
	Label *label;
	Scale *slider;
	Separator *sep;
	Button *stop;

	bool continue_working;

	void createInterface(const string &label_format, int total);

    private:
	bool		working;
};

#endif
