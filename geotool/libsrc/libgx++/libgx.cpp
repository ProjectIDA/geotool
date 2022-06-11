#include "motif++/Application.h"
#include "WaveformWindow.h"
#include "ColorSelection.h"
#include "ColorTable.h"
#include "CSSTable.h"
#include "Filter.h"
#include "FrameTable.h"
#include "MultiTable.h"
#include "TableQuery.h"

static void createColorSelection(const string &name, Component *parent,
			const string &params) {
    new ColorSelection(name, parent);
}
static void createColorTable(const string &name, Component *parent,
			const string &params) {
    new ColorTable(name, parent);
}
static void createCSSTable(const string &name, Component *parent,
			const string &params) {
    new CSSTable(name, parent);
}
static void createFilter(const string &name, Component *parent,
			const string &params) {
    new Filter(name, parent);
}
static void createFrameTable(const string &name, Component *parent,
			const string &params) {
    new FrameTable(name, parent);
}
static void createMultiTable(const string &name, Component *parent,
			const string &params) {
    new MultiTable(name, parent);
}
static void createTableQuery(const string &name, Component *parent,
			const string &params) {
    new TableQuery(name, parent);
}
static void createTableViewer(const string &name, Component *parent,
			const string &params) {
    new TableViewer(name, parent);
}
static void createWaveformPlot(const string &name, Component *parent,
			const string &params) {
    new WaveformPlot(name, parent);
}
static void createWaveformView(const string &name, Component *parent,
			const string &params) {
    new WaveformView(name, parent);
}
static void createWaveformWindow(const string &name, Component *parent,
			const string &params) {
    new WaveformWindow(name, parent);
}

void registerLibgx(void)
{
    Application *app = Application::getApplication();
    app->addCreateMethod("ColorSelection", createColorSelection);
    app->addCreateMethod("ColorTable", createColorTable);
    app->addCreateMethod("CSSTable", createCSSTable);
    app->addCreateMethod("Filter", createFilter);
    app->addCreateMethod("FrameTable", createFrameTable);
    app->addCreateMethod("MultiTable", createMultiTable);
    app->addCreateMethod("TableQuery", createTableQuery);
    app->addCreateMethod("TableViewer", createTableViewer);
    app->addCreateMethod("WaveformPlot", createWaveformPlot);
    app->addCreateMethod("WaveformView", createWaveformView);
    app->addCreateMethod("WaveformWindow", createWaveformWindow);
}
