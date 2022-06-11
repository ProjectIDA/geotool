/** \file MapOverlayForm.cpp
 *  \brief Defines class MapOverlayForm.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <sstream>
#include <sys/param.h>
#include <dirent.h>
#include <errno.h>

#include "MapOverlay.h"
#include "MapOverlayForm.h"
#include "MapWindow.h"
#include "motif++/MotifClasses.h"

extern "C" {
#include "libstring.h"
}

using namespace libgmap;

MapOverlayForm::MapOverlayForm(const char *name, Component *parent,
		MapWindow *mapWindow) : FormDialog(name, parent, false, false)
{
    Arg args[20];
    int n;

    mw = mapWindow;
    map = mw->map;
    overlays = new gvector<MapOverlay *>;

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    label = new Label("Map Overlays", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNorientation, XmHORIZONTAL); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    controls = new RowColumn("controls", this, args, n);

    close_button = new Button("Close", controls, this);
    remove_button = new Button("Remove", controls, this);
    help_button = new Button("Help", controls, this);
    controls->setHelp(help_button);

    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, controls->baseWidget()); n++;
    sep = new Separator("sep", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNtopWidget, label->baseWidget()); n++;
    XtSetArg(args[n], XmNtopOffset, 5); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNbottomWidget, sep->baseWidget()); n++;
    XtSetArg(args[n], XmNbottomOffset, 5); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNleftOffset, 5); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightOffset, 5); n++;
    XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); n++;
    XtSetArg(args[n], XmNscrollBarPlacement, XmBOTTOM_LEFT); n++;
    sw = new ScrolledWindow("sw", this, args, n);

    n = 0;
    XtSetArg(args[n], XmNselectionPolicy, XmEXTENDED_SELECT); n++;
    XtSetArg(args[n], XmNshadowThickness, 0); n++;
    XtSetArg(args[n], XmNhighlightThickness, 0); n++;
    XtSetArg(args[n], XmNvisibleItemCount, 15); n++;
    XtSetArg(args[n], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE); n++;
    list = new List("list", sw, args, n);
    list->addActionListener(this, XmNextendedSelectionCallback);
    list->addActionListener(this, XmNmultipleSelectionCallback);

    readOverlayDir();
}

MapOverlayForm::~MapOverlayForm(void)
{
    delete overlays;
}

void MapOverlayForm::actionPerformed(ActionEvent *action_event)
{
    const char *cmd = action_event->getActionCommand();

    if(!strcmp(cmd, "list")) {
	overlaySelect();
    }
    else if(!strcmp(cmd, "Close")) {
	setVisible(false);
    }
    else if(!strcmp(cmd, "Remove")) {
	overlayRemove();
    }
    else if(!strcmp(cmd, "Help")) {
	showHelp("Map Overlay Help");
    }
}

ParseCmd MapOverlayForm::parseCmd(const string &cmd, string &msg)
{
    char *tmp=NULL;
    string c;
    vector<string> items;
    vector<int> pos;

   if(parseCompare(cmd, "Help")) {
        char prefix[200];
        getParsePrefix(prefix, sizeof(prefix));
        parseHelp(prefix);
	return COMMAND_PARSED;
    }
    else if(parseArg(cmd, "overlays", c)) {
	tmp = strdup(c.c_str());
    }
    else {
	tmp = strdup(cmd.c_str());
    }
    if( !strcasecmp(tmp, "none") ) {
	list->deselectAll();
	overlaySelect();
	free(tmp);
	return COMMAND_PARSED;
    }

    list->getItems(items);

    char *last, *s, *tok = tmp;
    while( (s = strtok_r(tok, " ,\t", &last)) )
    {
	tok = NULL;
	for(int i = 0; i < (int)items.size(); i++) {
	    if((int)pos.size() < (int)items.size() && !items[i].compare(s)) {
		pos.push_back(i+1);
	    }
	}
    }
    free(tmp);

    if((int)pos.size() > 0) {
	list->selectOnly(pos, true);
	return COMMAND_PARSED;
    }
    return FormDialog::parseCmd(cmd, msg);
}

void MapOverlayForm::parseHelp(const char *prefix)
{
    printf("%soverlays=OVERLAY,OVERLAY...\n", prefix);
    printf("%soverlays=none\n", prefix);
}

void MapOverlayForm::readOverlayDir(void)
{
    string prop;
    if(getProperty("mapOverlayDir", prop)) {
        readOverlayDir(prop.c_str(), false);
    }
    else {
	const char *c;
        char path[MAXPATHLEN+1];
	if((c = Application::getInstallDir("GEOTOOL_HOME"))) {
            snprintf(path, sizeof(path), "%s/tables/mapoverlays", c);
            readOverlayDir(path, false);
        }
    }
}

void MapOverlayForm::readOverlayDir(const string &overlay_dir, bool warn)
{
    DIR *dirp;
    struct dirent *dp;
    ostringstream os;

    if((dirp = opendir(overlay_dir.c_str())) == NULL)
    {
        if(!warn) return;
        if(errno > 0) {
            showWarning("Cannot open: %s\n%s", overlay_dir.c_str(),
			strerror(errno));
        }
        else {
            showWarning("Cannot open: %s", overlay_dir.c_str());
        }
        return;
    }
    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
        if(dp->d_name[0] != '.')
        {
	    os.str("");
	    os << overlay_dir << "/" << dp->d_name;
/*
            if( (fp = fopen(path, "r")) ) {
                char ovr[8];
                ovr[7] = '\0';
                if(fread(ovr, 1, 7, fp) == 7 && !strcasecmp(ovr, "overlay")) {
                    listOverlay(new MapOverlay(path, mw));
                }
                fclose(fp);
            }
*/
	    try {
	 	listOverlay(new MapOverlay(os.str(), mw));
	    }
	    catch(...) {}
        }
    }
    closedir(dirp);
}

void MapOverlayForm::overlaySelect(void)
{
    int	i, j, num, *pos = NULL;
    bool redisplay = false;

    num = list->getSelectedPos(&pos);

    for(i = overlays->size()-1; i >= 0; i--)
    {
	MapOverlay *o = overlays->at(i);
	for(j = 0; j < num; j++) if(pos[j] == i+1) break;
	if(j < num && !o->selected)
	{
	    redisplay = true;
	    o->selected = true;
	    if(!o->has_been_read)
	    {
		o->readOverlay(MAP_ON);
	    }
	    map->display(o->num_ids, o->id, MAP_LOCKED_ON, false);
	}
	else if(j == num && o->selected)
	{
	    redisplay = true;
	    o->selected = false;
	    map->display(o->num_ids, o->id, MAP_LOCKED_OFF, false);
	}
    }
    if(redisplay) {
	map->update();
    }
    XtFree((char *)pos);
}

void MapOverlayForm::overlayRemove(void)
{
    int	 i, j, k, num, *pos = NULL;
    bool redisplay = false;

    num = list->getSelectedPos(&pos);

    for(i = overlays->size()-1; i >= 0; i--)
    {
	MapOverlay *o = overlays->at(i);
	for(j = 0; j < num; j++) if(pos[j] == i+1) break;
	if(j < num)
	{
	    redisplay = true;
	    for(k = 0; k < o->num_ids; k++) {
		map->mapDelete(o->id[k], false);
	    }
	    overlays->removeAt(i);
	}
    }
    if(redisplay)
    {
	list->deselectAll();
	list->deleteAll();
	const char **s = (const char **)malloc(overlays->size()*sizeof(char *));
	for(i = 0; i < overlays->size(); i++)
        {
	    MapOverlay *o = overlays->at(i);
	    s[i] = quarkToString(o->path);
        }
	list->addItems(s, overlays->size(), 1);
        Free(s);

	map->update();
    }
    XtFree((char *)pos);
}

void MapOverlayForm::listOverlay(MapOverlay *o)
{
    if(!overlays->contains(o) && !duplicatePath(o)) {
	overlays->add(o);
	addOverlayName(o);
    }
}

void MapOverlayForm::displayOverlay(MapOverlay *o)
{
    int i;
    for(i = 0; i < overlays->size() && o != overlays->at(i); i++);
    if(i == overlays->size()) {
	if(!duplicatePath(o)) {
	    overlays->add(o);
	    addOverlayName(o);
	    list->selectPos(0, true);
	}
	else {
	    for(i = 0; i < overlays->size(); i++) {
		if(overlays->at(i) == (Gobject *)o) {
		    list->selectPos(i+1, true);
		    break;
		}
	    }
	}
    }
    else {
	list->selectPos(i+1, true);
    }
}

bool MapOverlayForm::duplicatePath(MapOverlay *o_new)
{
    for(int i = 0; i < overlays->size(); i++)
    {
	MapOverlay *o = overlays->at(i);
	if(o->path == o_new->path)
	{
	    for(int j = 0; j < o->num_ids; j++) {
		map->mapDelete(o->id[j], false);
	    }
	    overlays->set(o_new, i);
	    map->update();
	    return true;
	}
    }
    return false;
}

void MapOverlayForm::addOverlayName(MapOverlay *o)
{
    short count;
    Arg args[1];

    /* add o->name to an XmList widget.
     */
    list->addItem((char *)quarkToString(o->name), 0);
    count = overlays->size();
    XtSetArg(args[0], XmNvisibleItemCount, count);
    list->setValues(args, 1);

    scrollRight();
}

void MapOverlayForm::scrollRight(void)
{
    Arg     args[2];
    Widget  sb;
    int     max;
    int     value;
    int     slider_size;
    XmScrollBarCallbackStruct call_value;

    XtSetArg(args[0], XmNhorizontalScrollBar, &sb);
    sw->getValues(args, 1);
    if(!sb) {
	return;
    }

    XtSetArg(args[0], XmNmaximum, &max);
    XtSetArg(args[1], XmNsliderSize, &slider_size);
    XtGetValues(sb, args, 2 );

    value = max - slider_size;

    XtSetArg(args[0], XmNvalue, value);
    XtSetValues(sb, args, 1 );

    /* fake a drag event on the scroll bar to actually scroll the window */
    call_value.reason = XmCR_DRAG;
    call_value.event = NULL;
    call_value.value = value;
    XtCallCallbacks(sb, XmNdragCallback, &call_value);
}

bool MapOverlayForm::isOverlayId(int id)
{
    for(int i = 0; i < overlays->size(); i++) {
	MapOverlay *o = overlays->at(i);
	for(int j = 0; j < o->num_ids; j++) {
	    if(o->id[j] == id) return true;
	}
    }
    return false;
}
