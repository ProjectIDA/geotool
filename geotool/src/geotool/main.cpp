#include "config.h"
#include "motif++/Application.h"
#include "libgdb.h"
#include "WaveformWindow.h"

static char *getPropFile(char *window, char *filename, int len);
static void cleanUp(void);
static void registerCreateMethods(void);

int
main(int argc, const char **argv)
{
    extern char Versionstr[];
    extern char installation_directory[];
    Application *app;
    char *window = NULL, cmd[500];

    // check the command line for the argument "window="
    window = stringGetArgv(argc, argv, "window");

    app = new Application("geotool", &argc, argv, cleanUp, Versionstr,
				installation_directory);
    registerCreateMethods();

    app->setIconRoutine(WaveformWindow::makeIcon);

    if( !window ) {
	app->readApplicationProperties(".geotool++/geotool");
    }
    else {
	app->readApplicationProperties(getPropFile(window, cmd, sizeof(cmd)));
    }

    app->setDefaultResources();

    WaveformWindow *ww = new WaveformWindow("geotool", app);

    if( !window ) {
	ww->setVisible(true); // The WaveformWindow is the first window
    }
    else {
	ParseCmd ret;
	// Another window is the first window
	snprintf(cmd, sizeof(cmd), "%s.open", window);
	if((ret = ww->putCmd(cmd)) != COMMAND_PARSED) {
	    if(ret == COMMAND_NOT_FOUND) {
		cerr << window << " window not found." << endl;
	    }
	    return(1);
	}
	ww->changeWindowMenu(window);
	Free(window);
    }

    app->handleEvents();

    return 0;
}

static char *
getPropFile(char *window, char *filename, int len)
{
    char s[500];
    snprintf(s, sizeof(s), "%s", window);
    for(int i = 0; i < (int)strlen(s); i++) {
	if( !isspace((int)s[i]) ) {
	    s[i] = tolower((int)s[i]);
	}
	else {
	    s[i] = '_';
	}
    }
    snprintf(filename, len, ".geotool++/%s", s);
    return filename;
}

#include "libcssio.h"
#include "FFDatabase.h"

static void cleanUp(void)
{
    cssioDeleteAllTmp();
    FFDatabase::clearStaticTables();
#ifdef HAVE_LIBODBC
    ODBCCloseDatabase();
#endif
}

static void registerCreateMethods(void)
{
    extern void registerLibwgets(void);
    extern void registerLibgx(void);
    registerLibwgets();
    registerLibgx();
}
