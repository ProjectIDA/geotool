#include "motif++/MotifClasses.h"
#include "Example.h"

int
main(int argc, const char **argv)
{
    extern char Versionstr[];
    extern char installation_directory[];
    Application *app;

    app = new Application("example", &argc, argv, Versionstr,
				installation_directory);

    app->readApplicationProperties(".example");

    app->setDefaultResources();

    Example *ex = new Example("example", app);
    ex->setSize(400, 400);
    ex->setVisible(true);

    app->handleEvents();

    return 0;
}
