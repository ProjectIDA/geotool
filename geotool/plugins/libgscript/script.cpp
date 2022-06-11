/** \file script.cpp
 *  \brief Defines libgscript plugins.
 *  \author Ivan Henson
 */
#include "config.h"
#include <math.h>
#include "script.h"
#include "motif++/MotifClasses.h"
#include "Waveform.h"

using namespace libgscript;

static PlugIn *createPlugin(Application *app, TopWindow *tw_parent,
			bool create_button, DataSource *ds, DataReceiver *dr)
{
    return new gscriptPlugin(tw_parent, ds, dr);
}

static PlugInStruct plugins[] =
{
    // app_name, parent_class, parent_name, name, description, createFunction
    {NULL,  "WaveformWindow", NULL, "gsc", "Script Library", createPlugin},
};

extern "C" int plugInIndex(PlugInStruct **p)
{
    *p = plugins;
    return sizeof(plugins)/sizeof(PlugInStruct);
}


/** Parse gsc script commands. This method is called for all script commands
 *  that begin with "gsc.".
 *  @param[in] cmd The command line.
 *  @param[out] msg Returned message.
 *  @param[in] the length of msg.
 */
ParseCmd gscriptPlugin::parseCmd(const string &cmd, string &msg)
{
    if(parseCompare(cmd, "cmd1", 4)) {
	return parseCmd1(cmd, msg);
    }
    else if(parseCompare(cmd, "cmd2", 4)) {
	return parseCmd2(cmd, msg);
    }
    return COMMAND_NOT_FOUND;
}

/** Parse the command gsc.cmd1 wave[1] low= high= length=
 */
ParseCmd gscriptPlugin::parseCmd1(const string &cmd, string &msg)
{
    int id, length;
    double low, high;
    Waveform *w=NULL;

    // get all arguments
    if( !parseGetArg(cmd, "cmd1", msg, "_wave_", &id) ) {
	msg.assign("cmd1: missing wave object");
	return ARGUMENT_ERROR;
    }
    if( !(w = ds->getWaveform(id)) ) {
	msg.assign("cmd1: wave object not found");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(cmd, "cmd1", msg, "low", &low) ) {
	msg.assign("cmd1: missing low argument");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(cmd, "cmd1", msg, "high", &high) ) {
	msg.assign("cmd1: missing high argument");
	return ARGUMENT_ERROR;
    }
    if( !parseGetArg(cmd, "cmd1", msg, "length", &length) ) {
	msg.assign("cmd1: missing length argument");
	return ARGUMENT_ERROR;
    }

    // do something with w and arguments
    printf("wave = %s/%s %.2f low=%.4g  high=%.4g  length=%d\n",
		w->sta(), w->chan(), w->tbeg(), low, high, length);

    return COMMAND_PARSED;
}

/** Parse the command gsc.cmd2
 */
ParseCmd gscriptPlugin::parseCmd2(const string &cmd, string &msg)
{
    int num, m;
    double amp;
    gvector<Waveform *> wvec;

    if((num = ds->getWaveforms(wvec)) == 0) {
	msg.assign("cmd2: no waveforms found");
	return ARGUMENT_ERROR;
    }

    // do something with all waveforms
    for(int i = 0; i < num; i++) {
	printf("%s/%s %.2f %.2f %s\n", wvec[i]->sta(), wvec[i]->chan(),
		wvec[i]->lat(), wvec[i]->lon(), wvec[i]->instype());

	// compute the average amplitude of the first segment
        amp = 0.;
	m = wvec[i]->segment(0)->length();
	for(int j = 0; j < m; j++) {  
	    amp += fabs(wvec[i]->segment(0)->data[j]);
	}
	if(m > 0) amp /= m;
	if(i < 20) a[i] = amp;
    }

    return COMMAND_PARSED;
}

/** Parse gsc script variables. This method is called for all script variables
 *  that begin with "gsc.".
 *  @param[in] name The variable name.
 *  @param[out] value Returned variable value.
 *  @param[in] the length of value.
 */
ParseVar gscriptPlugin::parseVar(const string &name, string &value)
{
    ParseVar ret;

    if(parseArray(name, "a", 20, a, value, &ret)) {
	// parseArray takes care of it
	return ret;
    }
    else if(parseCompare(name, "b")) {
	parsePrintDouble(value, b);
    }
    else if(parseCompare(name, "c")) {
	parsePrintFloat(value, c);
    }
    else if(parseCompare(name, "n")) {
	parsePrintInt(value, n);
    }
    else {
	return VARIABLE_NOT_FOUND;
    }
    return STRING_RETURNED;
}
