#include "motif++/Application.h"
#include "widget/ConPlotClass.h"
#include "widget/CPlotClass.h"
#include "widget/Map.h"
#include "widget/Table.h"
#include "widget/TabClass.h"

static void createAxesClass(const string &name, Component *parent,
		const string &params)
{
    new AxesClass(name, parent);
}
static void createConPlotClass(const string &name, Component *parent,
		const string &params)
{
    new ConPlotClass(name, parent);
}
static void createCPlotClass(const string &name, Component *parent,
		const string &params)
{
    new CPlotClass(name, parent);
}
static void createMap(const string &name, Component *parent,
		const string &params)
{
    new Map(name, parent);
}
static void createTabClass(const string &name, Component *parent,
		const string &params)
{
    new TabClass(name, parent);
}
static void createTable(const string &name, Component *parent,
		const string &params)
{
    new Table(name, parent);
}

void registerLibwgets(void)
{
    Application *app = Application::getApplication();
    app->addCreateMethod("AxesClass", createAxesClass);
    app->addCreateMethod("ConPlotClass", createConPlotClass);
    app->addCreateMethod("CPlotClass", createCPlotClass);
    app->addCreateMethod("Map", createMap);
    app->addCreateMethod("TabClass", createTabClass);
    app->addCreateMethod("Table", createTable);
}
