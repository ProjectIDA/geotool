#ifndef _TEXT_QUESTION_H_
#define _TEXT_QUESTION_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"


/** A dialog window that displays a prompt, a text field for the answer and
 *  two buttons. The following example code creates the TextQuestion window
 *  shown below. The getAnswer function does not return until either one of
 *  the buttons has been selected or the window has been closed. If the first
 *  button is selected (in this case "Apply"), a string value is returned.
 *  If the second button is selected (Cancel) or the window is closed, then
 *  NULL is returned.
 *  \code
 *  TextQuestion *q = new TextQuestion("Rename Arrival", this, "Enter new name", "Apply", "Cancel");
 *  char *new_name = q->getAnswer();
 *  q->destroy();
 *  \endcode
 *  \image html TextQuestion.gif "The TextQuestion window"
 *  \image latex TextQuestion.eps "The TextQuestion" width=3in
 *  @ingroup libmotif
 */
class TextQuestion : public FormDialog
{
    public:
	TextQuestion(const string &name, Component *parent,
		const string &question, const string &button1_label,
		const string &button2_label);
	~TextQuestion(void);

	char *getAnswer(void) {
	    string ans;
	    if(getAnswer(ans)) return strdup(ans.c_str());
	    return NULL;
	}
	bool getAnswer(string &ans);

	void actionPerformed(ActionEvent *action_event);
	static bool askTextQuestion(const string &name, Component *parent,
		const string &question, string &ans,
		const string &button1_label,
		const string &button2_label);
	static bool askTextQuestion(const string &name, Component *parent,
		const string &question, string &ans,
		const string &default_ans,
		const string &button1_label,
		const string &button2_label);

	static char *askTextQuestion(const string &name, Component *parent,
		const string &question, const string &button1_label,
		const string &button2_label)
	{
	    string ans;
	    if(askTextQuestion(name, parent, question, ans, button1_label,
			button2_label)) return strdup(ans.c_str());
	    return NULL;
	}
	static char *askTextQuestion(const string &name, Component *parent,
		const string &question, const string &default_ans,
		const string &button1_label, const string &button2_label)
	{
	    string ans;
	    if(askTextQuestion(name, parent, question, ans, default_ans,
		button1_label, button2_label)) return strdup(ans.c_str());
	    return NULL;
	}

    protected:
	int answer;
	Button *b1, *b2;
	Label *title, *label;
	Separator *sep1, *sep2;
	RowColumn *controls;
	TextField *text;

	void createInterface(const string &question, const string &button1,
			const string &button2);

    private:
};

#endif
