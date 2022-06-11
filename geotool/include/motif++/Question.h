#ifndef _QUESTION_H_
#define _QUESTION_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifDecs.h"

/** A dialog window that displays a message and one, two, or three buttons for a
 *  response. An example program that illustrates the usage is show below.
 *  \include QuestionExample.cpp
 *  @ingroup libmotif
 */
class Question : public FormDialog
{
    public:
	Question(const string &name, Component *parent, const string &question,
		const string &button1="", const string &button2="",
		const string &button3="");
	~Question(void);

	int getAnswer(void);

	void actionPerformed(ActionEvent *action_event);
	static int askQuestion(const string &name, Component *parent,
		const string &question, const string &button1="",
		const string &button2="", const string &button3="");

    protected:
	int answer;
	Button *b1, *b2, *b3;
	Label *title, *label;
	Separator *sep1, *sep2;
	RowColumn *controls;

	void createInterface(const string &, const string &, const string &,
			const string &);

    private:
};

#endif
