#ifndef _IMPORT_SEED_STATION_H_
#define _IMPORT_SEED_STATION_H_

#include "motif++/FormDialog.h"
#include "motif++/MotifClasses.h"

/**
 *  @ingroup libgx
 */
class ImportSeedStation : public FormDialog
{
  public:

    ImportSeedStation(const string &name, Component *parent, ActionListener *listener);
    ~ImportSeedStation(void);

    void actionPerformed(ActionEvent *action_event);

    void setSeedFile(string file);

//  ParseCmd parseCmd(const string &cmd, string &msg);
//    static void parseHelp(const char *prefix);

 protected:

    string seed_file;
    Label *file_label;
    Label *dir_label;
    Label *prefix_label;
    Label *response_label;
    TextField *dir_text;
    TextField *prefix_text;
    TextField *response_text;
    RowColumn *rc, *controls;
    RadioBox *rb;
    Toggle *update_toggle, *overwrite_toggle;
    Separator *sep;
    Button *import_button, *cancel_button;

    void createInterface(void);
    void setDefaultFiles(void);
    void import(void);
    void cancel(void);
    bool dirOk(string name, string dir);
    void setTableFiles2(string dir, string prefix, string respdir);
    bool mkpath(string path);
};

#endif
