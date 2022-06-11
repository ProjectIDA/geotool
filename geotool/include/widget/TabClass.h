#ifndef TABCLASS_H
#define TABCLASS_H

#include "motif++/Component.h"
extern "C" {
#include "widget/Tab.h"
}

/** This class is the interface to the Tab Widget.
 *
 *  @ingroup libwgets
 */
class TabClass : public Component
{
    public:

	TabClass(const string &name, Component *parent);
	TabClass(const string &name, Component *parent, Arg *args, int n);
	~TabClass(void);

	void destroy(void);
	int numTabs(void) { return TabNumTabs(tw); }
	int getLabels(char ***tabLabels) {return TabGetLabels(tw, tabLabels);}
	void setLabel(const string &oldLabel, const string &newLabel) {
		TabSetLabel(tw, oldLabel.c_str(), newLabel.c_str()); }
	void setLabel(int i, const string &newLabel) {
		TabSetTabLabel(tw, i, newLabel.c_str());
	}
	void setTabColor(const string &label, Pixel fg) {
		TabSetTabColor(tw, label.c_str(), fg);
	}
        void removeTabColors(void) { TabRemoveTabColors(tw); }
	int deleteTab(const string &name) { return TabDelete(tw, name.c_str());}
	int deleteTab(int i) {
	    char **labels=NULL;
	    int ret=0, num = TabGetLabels(tw, &labels);
	    if(i >= 0 && i < num) {
		ret = TabDelete(tw, labels[i]);
	    }
	    Free(labels);
	    return ret;
	}
	void orderTabs(int num, char **orderedLabels) {
		TabOrder(tw, num, orderedLabels); }
	Widget getTab(const string &tabLabel) {
		return TabGetTab(tw, tabLabel.c_str()); }
	Widget getTabOnTop(void) { return TabOnTop(tw); }
	char *labelOnTop(void) { return TabLabelOnTop(tw); }
	void setOnTop(const string &tabLabel) {
		TabSetOnTop(tw, tabLabel.c_str());}
	void setOnTop(int i) {
	    char **labels=NULL;
	    int num = TabGetLabels(tw, &labels);
	    if(i >= 0 && i < num) {
		TabSetOnTop(tw, labels[i]);
	    }
	    Free(labels);
	}
	void setSensitive(const string &tabLabel, bool state) {
		TabSetSensitive(tw, tabLabel.c_str(), (Boolean)state); }
	void setSensitive(int i, bool state) {
	    char **labels=NULL;
	    int num = TabGetLabels(tw, &labels);
	    if(i >= 0 && i < num) {
		TabSetSensitive(tw, labels[i], (Boolean)state);
	    }
	    Free(labels);
	}
	Size getPreferredsize(void) { return TabGetPreferredSize(tw); }
	void deleteAllTabs(void) { TabDeleteAllTabs(tw); }
	void update(void) { TabUpdate(tw); }
	bool mapChild(Component *child) {
	    return (bool)TabMapChild(tw, child->baseWidget()); }
	bool unmapChild(Component *child) {
	    return (bool)TabUnmapChild(tw, child->baseWidget()); }

    protected:
	TabWidget tw;

	void init(void);

    private:

	static void tabCallback(Widget, XtPointer, XtPointer);
	static void tabMenuCallback(Widget, XtPointer, XtPointer);
	static void insensitiveTabCallback(Widget, XtPointer, XtPointer);
};

#endif
