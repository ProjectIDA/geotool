/** \file Parser.cpp
 *  \brief Defines class Parser.
 *  \author Ivan Henson
 */
#include "config.h"
#include "widget/Parser.h"
#include "motif++/Parse.h"
#include "motif++/Application.h"
using namespace Parse;

extern "C" {
#include "libstring.h"
}

static bool parseIndex(const char *c, int nsegs, int *i);
static void printWave(Waveform *w, string &msg);
static void printSegment(Waveform *w, int i, string &msg);
static void printTable(CssTableClass *o, string &msg);

Parser::Parser(void) : DataSource()
{
}

Parser::~Parser(void)
{
    for(int i = 0; i < (int)tps.size(); i++) {
	delete tps[i];
    }
    tps.clear();
}

ParseVar Parser::dataParse(const string &name, string &value)
{
    ParseVar ret;

    if((ret = getWaveformLoop(name.c_str(), value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if((ret = getTableLoop(name.c_str())) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if((ret = getWaveMember(name, value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    else if((ret = getTableMember(name.c_str(), value)) != VARIABLE_NOT_FOUND){
	return ret;
    }
    else if((ret = getWaveformCmd(name.c_str(), value)) != VARIABLE_NOT_FOUND){
	return ret;
    }
    else if((ret = getTableCmd(name.c_str(), value)) != VARIABLE_NOT_FOUND) {
	return ret;
    }
    return VARIABLE_NOT_FOUND;
}

ParseVar Parser::getWaveMember(const string &name, string &value)
{
    ostringstream os;
    int i, num, nextc;
    gvector<Waveform *> wvec;
    ParseVar ret;

    if(!parseCompare(name, "wave", 4) && !parseCompare(name, "sel_wave", 8)) {
	return VARIABLE_NOT_FOUND;
    }
    else if(parseCompare(name, "wave.size_")) {
	os << getWaveforms(wvec);
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "sel_wave.size_")) {
	os << getSelectedWaveforms(wvec);
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(parseCompare(name, "wave.find_indices_", 18)) {
	return waveFindIndices(name.c_str(), value);
    }

    num = getWaveforms(wvec);
    if(!parseArrayIndex(name, "wave", num, &i, &nextc, value, &ret)) {
	num = getSelectedWaveforms(wvec);
	if(!parseArrayIndex(name, "sel_wave", num, &i, &nextc, value, &ret))
	{
	    return ret;
	}
    }

    if(ret == STRING_RETURNED) {
	if(nextc >= (int)name.length()) {
	    os << "_wave_=" << wvec[i]->getId();
	    value.assign(os.str());
	}
	else {
	    ret = getTSMember(name.c_str()+nextc, wvec[i], value);
	}
    }
    return ret;
}

ParseVar Parser::getTableMember(const char *name, string &value)
{
    const char **table_names=NULL;
    char ndex[20], member[100], tnam[100];
    const char *c, *e;
    ostringstream os;
    long addr;
    int i, n=0, num=0, num_css;
    gvector<CssTableClass *> css;
    CssTableClass *t;
    bool selected, b;

    num_css = CssTableClass::getAllNames(&table_names);
    if(!strncmp(name, "sel_", 4) && strstr(name, ".size_")) {
	for(i = 0; i < num_css; i++) {
	    snprintf(tnam, sizeof(tnam), "sel_%s.size_", table_names[i]);
	    if(!strcasecmp(name, tnam)) {
		os << getSelectedTable(table_names[i], css);
		value.assign(os.str());
		Free(table_names);
		return STRING_RETURNED;
	    }
	}
	Free(table_names);
	return VARIABLE_NOT_FOUND;
    }
    else if(strstr(name, ".size_")) {
	for(i = 0; i < num_css; i++) {
	    snprintf(tnam, sizeof(tnam), "%s.size_", table_names[i]);
	    if(!strcasecmp(name, tnam)) {
		os << getTable(table_names[i], css);
		value.assign(os.str());
		Free(table_names);
		return STRING_RETURNED;
	    }
	}
	Free(table_names);
	return VARIABLE_NOT_FOUND;
    }
    else if(!strncmp(name, "sel_", 4)) {
	for(i = 0; i < num_css; i++) {
	    n = (int)strlen(table_names[i]);
	    if(!strncasecmp(name+4, table_names[i], n) && name[4+n] == '[') {
		break;
	    }
	}
	if(i < num_css) strncpy(tnam, table_names[i], sizeof(tnam)-1);
	Free(table_names);
	if(i == num_css) return VARIABLE_NOT_FOUND;
	selected = true;
	c = name+5+n;
    }
    else if(!strncmp(name, "modified_table.", 15)) {
	Application *app = Application::getApplication();
	if( (t = app->getModifiedTable()) ) {
	    b = getTabMember(name+15, t, value);
	    return b ? STRING_RETURNED : VARIABLE_NOT_FOUND;
	}
	return VARIABLE_NOT_FOUND;
    }
    else if(!strcmp(name, "modified_table")) {
	Application *app = Application::getApplication();
	if( (t = app->getModifiedTable()) ) {
	    addr = (long)t;
	    os << "_" << t->getName() << "_=" << addr;
	    value.assign(os.str());
	    return STRING_RETURNED;
	}
	return VARIABLE_NOT_FOUND;
    }
    else {
	for(i = 0; i < num_css; i++) {
	    n = (int)strlen(table_names[i]);
	    if(!strncasecmp(name, table_names[i], n) && name[n] == '[') {
		break;
	    }
	}
	if(i < num_css) strncpy(tnam, table_names[i], sizeof(tnam)-1);
	Free(table_names);
	if(i == num_css) return VARIABLE_NOT_FOUND;
	selected = false;
	c = name+n+1;
    }

    while(*c != '\0' && isspace((int)*c)) c++;
    if(*c == '\0') {
	cerr << "variable syntax error: "<< name << endl;
	return VARIABLE_ERROR;
    }
    e = c+1;
    while(*e != '\0' && *e != ']' && !isspace((int)*e)) e++;
    if(*e == '\0' || e-c > (int)sizeof(ndex)-1) {
	cerr << "variable syntax error: "<< name << endl;
	return VARIABLE_ERROR;
    }
    strncpy(ndex, c, e-c);
    ndex[e-c] = '\0';
    
    if(!stringToInt(ndex, &i) || i <= 0) {
	cerr << "Invalid index: " << name << endl;
	return VARIABLE_ERROR;
    }

    c = e;
    if(*c != ']') {
	c++;
	while(*c != '\0' && *c != ']') c++;
	if(*c == '\0') {
	    cerr << "variable syntax error: "<< name << endl;
	    return VARIABLE_ERROR;
	}
    }
    c++;
    if(*c == '\0') {
	if(!selected) {
	    num = getTable(tnam, css);
	}
	else {
	    num = getSelectedTable(tnam, css);
	}
	if(i > num) {
	    cerr << tnam << " index "<< i << ">" << num << endl;
	    return VARIABLE_ERROR;
	}
	addr = (long)css[i-1];
	os << "_" << tnam << "_=" << addr;
	value.assign(os.str());
	return STRING_RETURNED;
    }

    if(*c != '.' || *(c+1) == '\0') {
	cerr << "variable.member syntax error: "<< name << endl;
	return VARIABLE_ERROR;
    }
    c++;
    e = c;
    while(*e != '\0' && !isspace(*e)) e++;
    if(e-c > (int)sizeof(member)-1) {
	cerr << "variable.member syntax error: "<< name << endl;
	return VARIABLE_ERROR;
    }
    strncpy(member, c, e-c);
    member[e-c] = '\0';

    if(!selected) {
	num = getTable(tnam, css);
    }
    else {
	num = getSelectedTable(tnam, css);
    }
    if(i > num) {
	cerr << tnam << " index "<< i << ">" << num << endl;
	return VARIABLE_ERROR;
    }

    b = getTabMember(member, css[i-1], value);

    return b ? STRING_RETURNED : VARIABLE_NOT_FOUND;
}

ParseVar Parser::getWaveformLoop(const char *name, string &value)
{
    const char *cmd="start", *nam;
    bool selected=false;
    WaveformParseStruct *wp=&wps;

    if(!strncasecmp(name, "foreach_start_wave:", 19)) {
	nam = name+19;
    }
    else if(!strncasecmp(name, "foreach_next_wave:", 18)) {
	nam = name+18;
	cmd = "next";
    }
    else if(!strncasecmp(name, "foreach_stop_wave:", 18)) {
	nam = name+18;
	cmd = "stop";
    }
    else if(!strncasecmp(name, "foreach_start_sel_wave:", 23)) {
	nam = name+23;
	selected = true;
	cmd = "start";
	wp = &wps_sel;
    }
    else if(!strncasecmp(name, "foreach_next_sel_wave:", 22)) {
	nam = name+22;
	cmd = "next";
	wp = &wps_sel;
    }
    else if(!strncasecmp(name, "foreach_stop_sel_wave:", 22)) {
	nam = name+22;
	cmd = "stop";
	wp = &wps_sel;
    }
    else {
	return VARIABLE_NOT_FOUND;
    }

    if(!strcasecmp(cmd, "start")) {
	strncpy(wp->name, nam, sizeof(wp->name));
	if(!selected) {
	    wp->num = getWaveforms(wp->wvec);
	}
	else {
	    wp->num = getSelectedWaveforms(wp->wvec);
	}
	wp->i = 0;
	return (wp->num > 0) ? FOREACH_MORE : FOREACH_NO_MORE;
    }
    else if(!strcasecmp(cmd, "next")) {
	wp->i++;
	return (wp->i < wp->num) ? FOREACH_MORE : FOREACH_NO_MORE;
    }
    else if(!strcasecmp(cmd, "stop")) {
	wp->num = 0;
	wp->i = 0;
	memset(wp->name, 0, sizeof(wp->name));
	return STRING_RETURNED;
    }
    else {
	return VARIABLE_ERROR;
    }
}

ParseVar Parser::getWaveformCmd(const char *name, string &value)
{
    char *c;
    ostringstream os;

    if(wps.name && !strcmp(wps.name, name)) {
	if(wps.i >= wps.num) {
            value.assign("invalid waveform index");
	    return VARIABLE_ERROR;
	}
	os << "_wave_=" << wps.wvec[wps.i]->getId();
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(wps_sel.name && !strcmp(wps_sel.name, name)) {
	if(wps_sel.i >= wps_sel.num) {
            value.assign("invalid waveform index");
	    return VARIABLE_ERROR;
	}
	os << "_wave_=" << wps_sel.wvec[wps_sel.i]->getId();
	value.assign(os.str());
	return STRING_RETURNED;
    }
    else if(wps.name && stringArg(name, wps.name, &c)) {
	if(wps.i >= wps.num) {
            value.assign("invalid waveform index");
	    return VARIABLE_ERROR;
	}
	return getTSMember(c, wps.wvec[wps.i], value);
    }
    else if(wps_sel.name && stringArg(name, wps_sel.name, &c)) {
	if(wps_sel.i >= wps_sel.num) {
            value.assign("invalid waveform index");
	    return VARIABLE_ERROR;
	}
	return getTSMember(c, wps_sel.wvec[wps_sel.i], value);
    }
    return VARIABLE_NOT_FOUND;
}

// static
ParseVar Parser::getTSMember(const char *c, Waveform *w, string &value)
{
    ostringstream os;
    ParseVar ret;
    GTimeSeries *ts = w->ts;
    GDataPoint *p;
    const char *s;
    double t;
    int i, id, nextc;

   if(*c == '.') c++;

    if(parseArrayIndex(c, "seg", w->size(), &i, &nextc, value, &ret)) {
	if(ret == STRING_RETURNED) {
	    if(c[nextc] == '\0') {
		id = w->getId();
		os << "_wave_seg_=" << id << "." << i;
		value.assign(os.str());
	    }
	    else {
		ret = getSegInfo(w->segment(i), c+nextc, value);
	    }
	}
	return ret;
    }

    if(!strncasecmp(c, "nearest", 7)) {
	if(!parseFuncArg(c, 1, "time", &t, value)){
	    return VARIABLE_ERROR;
	}
	p = ts->nearest(t);
	if(!strncasecmp(c, "nearestData_ ", 13)) {
	    parsePrintFloat(value, p->data());
	}
	else if(!strncasecmp(c, "nearestSegment_ ", 12)) {
	    parsePrintInt(value, p->segmentIndex()+1);
	}
	else if(!strncasecmp(c, "nearestIndex_ ", 14)) {
	    parsePrintInt(value, p->index()+1);
	}
	else {
	    delete p;
	    return VARIABLE_NOT_FOUND;
	}
	delete p;
	return STRING_RETURNED;
    }
    else if(!strncasecmp(c, "lowerBound", 10)) {
	if(!parseFuncArg(c, 1, "time", &t, value)){
	    return VARIABLE_ERROR;
	}
	p = ts->lowerBound(t);
	if(!strncasecmp(c, "lowerBoundData_ ", 16)) {
	    parsePrintFloat(value, p->data());
	}
	else if(!strncasecmp(c, "lowerBoundSegment_ ", 15)) {
	    parsePrintInt(value, p->segmentIndex()+1);
	}
	else if(!strncasecmp(c, "lowerBoundIndex_ ", 17)) {
	    parsePrintInt(value, p->index()+1);
	}
	else {
	    delete p;
	    return VARIABLE_NOT_FOUND;
	}
	delete p;
	return STRING_RETURNED;
    }
    else if(!strncasecmp(c, "upperBound", 10)) {
	if(!parseFuncArg(c, 1, "time", &t, value)){
	    return VARIABLE_ERROR;
	}
	p = ts->upperBound(t);
	if(!strncasecmp(c, "upperBoundData_ ", 16)) {
	    parsePrintFloat(value, p->data());
	}
	else if(!strncasecmp(c, "upperBoundSegment_ ", 15)) {
	    parsePrintInt(value, p->segmentIndex()+1);
	}
	else if(!strncasecmp(c, "upperBoundIndex_ ", 17)) {
	    parsePrintInt(value, p->index()+1);
	}
	else {
	    delete p;
	    return VARIABLE_NOT_FOUND;
	}
	delete p;
	return STRING_RETURNED;
    }
    else if(!strncasecmp(c, "time[", 5)) {
	if(!parseIndex(c+4, ts->length(), &i)) {
	    cerr << "Invalid time index: " << c << endl;
	    return VARIABLE_ERROR;
	}
	if( !(s = strstr(c, "]")) ) return VARIABLE_ERROR;

	parsePrintDouble(value, ts->time(i-1));
	return STRING_RETURNED;
    }
    if(!strcasecmp(c, "dataMin")) {
	parsePrintDouble(value, ts->dataMin());
    }
    else if(!strcasecmp(c, "dataMinSegment")) {
	p = ts->minPoint();
	parsePrintInt(value, p->segmentIndex()+1);
	delete p;
    }
    else if(!strcasecmp(c, "dataMinIndex")) {
	p = ts->minPoint();
	parsePrintInt(value, p->index()+1);
	delete p;
    }
    else if(!strcasecmp(c, "dataMax")) {
	parsePrintFloat(value, ts->dataMax());
    }
    else if(!strcasecmp(c, "dataMaxSegment")) {
	p = ts->maxPoint();
	parsePrintInt(value, p->segmentIndex()+1);
	delete p;
    }
    else if(!strcasecmp(c, "dataMaxIndex")) {
	p = ts->maxPoint();
	parsePrintInt(value, p->index()+1);
	delete p;
    }
    else if(!strcasecmp(c, "sta")) {
	value.assign(ts->sta());
    }
    else if(!strcasecmp(c, "chan")) {
	value.assign(ts->chan());
    }
    else if(!strcasecmp(c, "net")) {
	value.assign(ts->net());
    }
    else if(!strcasecmp(c, "refsta")) {
	value.assign(ts->refsta());
    }
    else if(!strcasecmp(c, "xchan")) {
	value.assign(ts->xChan());
    }
    else if(!strcasecmp(c, "ychan")) {
	value.assign(ts->yChan());
    }
    else if(!strcasecmp(c, "zchan")) {
	value.assign(ts->zChan());
    }
    else if(!strcasecmp(c, "instype")) {
	value.assign(ts->instype());
    }
    else if(!strcasecmp(c, "segtype")) {
	value.assign(ts->segtype());
    }
    else if(!strcasecmp(c, "datatype")) {
	value.assign(ts->datatype());
    }
    else if(!strcasecmp(c, "clip")) {
	value.assign(ts->clip());
    }
    else if(!strcasecmp(c, "chanid")) {
	parsePrintInt(value, ts->chanid());
    }
    else if(!strcasecmp(c, "lat")) {
	parsePrintDouble(value, ts->lat());
    }
    else if(!strcasecmp(c, "lon")) {
	parsePrintDouble(value, ts->lon());
    }
    else if(!strcasecmp(c, "elev")) {
	parsePrintDouble(value, ts->elev());
    }
    else if(!strcasecmp(c, "dnorth")) {
	parsePrintDouble(value, ts->dnorth());
    }
    else if(!strcasecmp(c, "deast")) {
	parsePrintDouble(value, ts->deast());
    }
    else if(!strcasecmp(c, "hang")) {
	parsePrintDouble(value, ts->hang());
    }
    else if(!strcasecmp(c, "vang")) {
	parsePrintDouble(value, ts->vang());
    }
    else if(!strcasecmp(c, "jdate")) {
	parsePrintInt(value, ts->jdate());
    }
    else if(!strcasecmp(c, "tbeg")) {
	parsePrintDouble(value, ts->tbeg());
    }
    else if(!strcasecmp(c, "tend")) {
	parsePrintDouble(value, ts->tend());
    }
    else if(!strcasecmp(c, "mean")) {
	parsePrintDouble(value, ts->mean());
    }
    else if(!strcasecmp(c, "duration")) {
	parsePrintDouble(value, ts->duration());
    }
    else if(!strcasecmp(c, "nsegs")) {
	parsePrintInt(value, ts->size());
    }
    else if(!strcasecmp(c, "length")) {
	parsePrintInt(value, ts->length());
    }
    else if(!strcasecmp(c, "samprate")) {
	parsePrintDouble(value, 1./ts->segment(0)->tdel());
    }
    else if(!strcasecmp(c, "calib")) {
	parsePrintDouble(value, ts->segment(0)->calib());
    }
    else if(!strcasecmp(c, "calper")) {
	parsePrintDouble(value, ts->segment(0)->calper());
    }
    else if(!strcasecmp(c, "index")) {
	parsePrintInt(value, w->default_order+1);
    }
    else if(!strcasecmp(c, "xpos")) {
	parsePrintDouble(value, w->scaled_x0);
    }
    else if(!strcasecmp(c, "ypos")) {
	parsePrintDouble(value, w->scaled_y0);
    }
    else if(!strcasecmp(c, "begSelect")) {
	parsePrintDouble(value, w->begSelect);
    }
    else if(!strcasecmp(c, "endSelect")) {
	parsePrintDouble(value, w->endSelect);
    }
    else if(!strcasecmp(c, "num_dw")) {
	parsePrintInt(value, w->num_dw);
    }
    else {
	char name[50];
	for(i = 0; i < w->num_dw; i++) {
	    snprintf(name, sizeof(name), "dw[%d].label", i+1);
	    if(!strcasecmp(c, name)) {
		value.assign(1, w->dw[i].label);
		return STRING_RETURNED;
	    }
	    snprintf(name, sizeof(name), "dw[%d].tbeg", i+1);
	    if(!strcasecmp(c, name)) {
		parsePrintDouble(value, w->dw[i].d1->time());
		return STRING_RETURNED;
	    }
	    snprintf(name, sizeof(name), "dw[%d].tend", i+1);
	    if(!strcasecmp(c, name)) {
		parsePrintDouble(value, w->dw[i].d2->time());
		return STRING_RETURNED;
	    }
	}
	for(i = 0; i < w->num_dp; i++) {
	    snprintf(name, sizeof(name), "dp[%d].label", i+1);
	    if(!strcasecmp(c, name)) {
		value.assign(1, w->dp[i]->getLabel());
		return STRING_RETURNED;
	    }
	    snprintf(name, sizeof(name), "dp[%d].time", i+1);
	    if(!strcasecmp(c, name)) {
		parsePrintDouble(value, w->dp[i]->time());
		return STRING_RETURNED;
	    }
	    snprintf(name, sizeof(name), "dp[%d].data", i+1);
	    if(!strcasecmp(c, name)) {
		parsePrintFloat(value, w->dp[i]->data());
		return STRING_RETURNED;
	    }
	}
	return VARIABLE_NOT_FOUND;
    }
    return STRING_RETURNED;
}

// static
ParseVar Parser::getSegInfo(GSegment *seg, const char *c, string &value)
{
    ParseVar ret;

    if(*c == '.') c++;

    if(parseArray(c, "data", seg->length(), seg->data, value, &ret))
    {
	return ret;
    }
    else if(!strcasecmp(c, "tbeg")) {
	parsePrintDouble(value, seg->tbeg());
    }
    else if(!strcasecmp(c, "tend")) {
	parsePrintDouble(value, seg->tend());
    }
    else if(!strcasecmp(c, "tdel")) {
	parsePrintDouble(value, seg->tdel());
    }
    else if(!strcasecmp(c, "samprate")) {
	parsePrintDouble(value, 1./seg->tdel());
    }
    else if(!strcasecmp(c, "length")) {
	parsePrintInt(value, seg->length());
    }
    else {
	return VARIABLE_NOT_FOUND;
    }
    return STRING_RETURNED;
}

static bool
parseIndex(const char *c, int nsegs, int *i)
{
    char ndex[30];
    const char *b, *e;
    int n;

    b = c+1;
    while(isspace((int)*b) && *b != '\0') b++;
    if(*b == '\0') return false;
    if( !(e = strstr(c, "]")) ) return false;
    e--;
    while(e > b && isspace((int)*e)) e--;
    if(e < b) return false;
    n = (int)(e-b) + 1;
    if(n > (int)sizeof(ndex)-1) return false;
    strncpy(ndex, b, n);
    ndex[n] = '\0';
    if(!stringToInt(ndex, i) || *i <= 0 || *i > nsegs) return false;
    *i = *i - 1;
    return true;
}

ParseVar Parser::getTableLoop(const char *name)
{
    char s[200];
    const char *cmd="start", *nam;
    const char **table_names=NULL, *table=NULL;
    int i, len=0, num_css;
    bool selected=false;
    TableParseStruct *tp=NULL;

    num_css = CssTableClass::getAllNames(&table_names);

    for(i = 0; i < num_css; i++) {
	snprintf(s, sizeof(s), "foreach_start_%s:", table_names[i]);
	len = (int)strlen(s);
	if(!strncasecmp(name, s, len)) {
	    break;
	}
	snprintf(s, sizeof(s), "foreach_start_sel_%s:", table_names[i]);
	len = (int)strlen(s);
	if(!strncasecmp(name, s, len)) {
	    selected = true;
	    break;
	}
	snprintf(s, sizeof(s), "foreach_next_%s:", table_names[i]);
	len = (int)strlen(s);
	if(!strncasecmp(name, s, len)) {
	    cmd = "next";
	    break;
	}
	snprintf(s, sizeof(s), "foreach_next_sel_%s:", table_names[i]);
	len = (int)strlen(s);
	if(!strncasecmp(name, s, len)) {
	    selected = true;
	    cmd = "next";
	    break;
	}
	snprintf(s, sizeof(s), "foreach_stop_%s:", table_names[i]);
	len = (int)strlen(s);
	if(!strncasecmp(name, s, len)) {
	    cmd = "stop";
	    break;
	}
	snprintf(s, sizeof(s), "foreach_stop_sel_%s:", table_names[i]);
	len = (int)strlen(s);
	if(!strncasecmp(name, s, len)) {
	    selected = true;
	    cmd = "stop";
	    break;
	}
    }
    if(i == num_css) {
	free(table_names);
	return VARIABLE_NOT_FOUND;
    }
    table = table_names[i];
    free(table_names);
    nam = name+len;

    for(i = 0; i < (int)tps.size(); i++)
    {
	if(!strcasecmp(tps[i]->table_name,table) && tps[i]->selected==selected)
	{
	    tp = tps[i];
	    break;
	}
    }
    if(!tp) {
	tp = new TableParseStruct();
	strncpy(tp->table_name, table, sizeof(tp->table_name));
	tp->selected = selected;
	tps.push_back(tp);
    }
    if(!strcmp(cmd, "start")) {
	memset(tp->name, 0, sizeof(tp->name));
	strncpy(tp->name, nam, sizeof(tp->name)-1);

	if(tp->selected) {
	    getSelectedTable(tp->table_name, tp->v);
	}
	else {
	    getTable(tp->table_name, tp->v);
	}
	tp->i = 0;
	return (tp->v.size() > 0) ? FOREACH_MORE : FOREACH_NO_MORE;
    }
    else if(!strcasecmp(cmd, "next")) {
	tp->i++;
	return(tp->i < tp->v.size()) ? FOREACH_MORE :FOREACH_NO_MORE;
    }
    else if(!strcasecmp(cmd, "stop")) {
	tp->v.clear();
	tp->i = 0;
	memset(tp->name, 0, sizeof(tp->name));
	memset(tp->table_name, 0, sizeof(tp->table_name));
	return STRING_RETURNED;
    }

    return VARIABLE_ERROR;
}

ParseVar Parser::getTableCmd(const char *name, string &value)
{
    char *c;
    ostringstream os;
    int i;
    long addr;

    for(i = 0; i < (int)tps.size(); i++) {
	if(tps[i]->name && !strcmp(tps[i]->name, name)) {
	    if(tps[i]->i >= tps[i]->v.size()) {
		value.assign(string("invalid ") +tps[i]->table_name+ " index");
		return VARIABLE_ERROR;
	    }
	    addr = (long)tps[i]->v[tps[i]->i];
	    os << "_" << tps[i]->table_name << "_=" << addr;
	    value.assign(os.str());
	    return STRING_RETURNED;
	}
	else if(stringArg(name, tps[i]->name, &c)) {
	    if(tps[i]->i >= tps[i]->v.size()) {
		value.assign(string("invalid ") +tps[i]->table_name+ " index");
		return VARIABLE_ERROR;
	    }
	    CssTableClass *css = tps[i]->v[tps[i]->i];
	    if( !getTabMember(c, css, value) ) {
		if(!strcasecmp(c, "index")) {
		    os << tps[i]->i+1;
		    value.assign(os.str());
		}
		return VARIABLE_NOT_FOUND;
	    }
	    return STRING_RETURNED;
	}
    }
    return VARIABLE_NOT_FOUND;
}

bool Parser::getTabMember(const char *c, CssTableClass *o, string &value)
{
    char  *member_address;
    int i;
    int num_members = o->getNumMembers();
    CssClassDescription *des = o->description();

    if(!strcasecmp(c, "tableName")) {
	value.assign(o->getName());
	return true;
    }
    else if(!strcasecmp(c, "selected")) {
	value.assign(isSelected(o) ? "true" : "false");
	return true;
    }

    // special case: get associated phase
    if(o->nameIs(cssArrival) && !strcasecmp(c, "phase"))
    {
	CssArrivalClass *a = (CssArrivalClass *)o;
	value.assign(a->phase);
	return true;
    }

    for(i = 0; i < num_members; i++)
    {
	if(!strcasecmp(c, des[i].name))
	{
	    member_address = (char *)o + des[i].offset;
	    switch(des[i].type)
	    {
	    case CSS_STRING:
		value.assign(member_address);
		break;
	    case CSS_QUARK:
		value.assign(quarkToString(*(int *)member_address));
		break;
	    case CSS_DATE:
	    case CSS_LDDATE:
		value.assign(timeDateString((DateTime *)member_address));
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		parsePrintInt(value, *(long *)member_address);
		break;
	    case CSS_INT:
		parsePrintInt(value, *(int *)member_address);
		break;
	    case CSS_DOUBLE:
	    case CSS_TIME:
		parsePrintDouble(value, *(double *)member_address);
		break;
	    case CSS_FLOAT:
		parsePrintFloat(value, *(float *)member_address);
		break;
	    default:
		value.assign("na");
            }

	    return true;
	}
    }
    return false;
}

ParseCmd Parser::dataParseCmd(const char *cmd, string &msg)
{
    int i, num_css;
    long id;
    Waveform *ws=NULL;
    char s[200], *c;
    gvector<CssTableClass *> css;
    const char **table_names=NULL;

    if(!strncasecmp(cmd, "printObject", 11)) {
	if(!stringGetLongArg(cmd, "_wave_", &id)) {
	    if( (ws = getWaveform(id)) ) {
		printWave(ws, msg);
		return COMMAND_PARSED;
	    }
	    else {
		msg.assign("Cannot find wave object.");
		return ARGUMENT_ERROR;
	    }
	}
	else if( (c = stringGetArg(cmd, "_wave_seg_")) ) {
	    if(sscanf(c, "%ld.%d", &id, &i) == 2 && (ws = getWaveform(id))
			&& i >= 0 && i < ws->size())
	    {
		printSegment(ws, i, msg);
		free(c);
		return COMMAND_PARSED;
	    }
	    else {
		msg.assign("Cannot find segment object.");
		free(c);
		return ARGUMENT_ERROR;
	    }
	}
	num_css = CssTableClass::getAllNames(&table_names);

	for(i = 0; i < num_css; i++) {
	    snprintf(s, sizeof(s), "_%s_", table_names[i]);
	    if(!stringGetLongArg(cmd, s, &id)) break;
	}
	if(i < num_css) {
	    getTable(table_names[i], css);
	    Free(table_names);
	    for(i = 0; i < css.size() && id != (long)css[i]; i++);
	    if(i < css.size()) {
		printTable(css[i], msg);
		return COMMAND_PARSED;
	    }
	}
	Free(table_names);
    }

    return COMMAND_NOT_FOUND;
}

static void
printWave(Waveform *ws, string &msg)
{
    GTimeSeries *ts = ws->ts;
    char s[100];
    GDataPoint *p;

#define snp(a,b) snprintf(s, sizeof(s), a, b); msg.append(s)
#define snp2(a,b,c) snprintf(s, sizeof(s), a, b, c); msg.append(s)

    msg.clear();
    snp("sta=%s\n", ts->sta());
    snp("chan=%s\n", ts->chan());
    snp("net=%s\n", ts->net());
    snp("lat=%.15g\n", ts->lat());
    snp("lon=%.15g\n", ts->lon());
    snp("elev=%.15g\n", ts->elev());
    snp("deast=%.15g\n", ts->deast());
    snp("dnorth=%.15g\n", ts->dnorth());
    snp("hang=%.15g\n", ts->hang());
    snp("vang=%.15g\n", ts->vang());
    snp("tbeg=%.15g\n", ts->tbeg());
    snp("tend=%.15g\n", ts->tend());
    snp("duration=%.15g\n", ts->duration());
    snp("nsegs=%d\n", ts->size());
    snp("length=%d\n", ts->length());
    snp("samprate=%.15g\n", 1./ts->segment(0)->tdel());
    snp("begSelect=%.15g\n", ws->begSelect);
    snp("endSelect=%.15g\n", ws->endSelect);
    snp("calib=%.15g\n", ts->segment(0)->calib());
    snp("calper=%.15g\n", ts->segment(0)->calper());
    snp("chanid=%d\n", ts->chanid());
    snp("clip=%s\n", ts->clip());
    snp("station_alpha=%.15g\n", ts->alpha());
    snp("station_beta=%.15g\n", ts->beta());
    snp("station_gamma=%.15g\n", ts->gamma());
    snp("current_alpha=%.15g\n", ts->currentAlpha());
    snp("current_beta=%.15g\n", ts->currentBeta());
    snp("current_gamma=%.15g\n", ts->currentGamma());
    snp("dataMax=%.7g\n", ts->dataMax());
    p = ts->maxPoint();
    snp("dataMaxIndex=%d\n", p->index()+1);
    snp("dataMaxSegment=%d\n", p->segmentIndex()+1);
    delete p;
    snp("dataMin=%.7g\n", ts->dataMin());
    p = ts->minPoint();
    snp("dataMinIndex=%d\n", p->index()+1);
    snp("dataMinSegment=%d\n", p->segmentIndex()+1);
    delete p;

    snp("datatype=%s\n", ts->datatype());
    snp("index=%d\n", ws->default_order+1);
    snp("instype=%s\n", ts->instype());
    snp("jdate=%ld\n", ts->jdate());
    snp("mean=%.15g\n", ts->mean());
    snp("num_dp=%d\n", ws->num_dp);
    for(int i = 0; i < ws->num_dp; i++) {
	snp2("dp[%d].label=%c\n", i+1, ws->dp[i]->getLabel());
	snp2("dp[%d].time=%.15g\n", i+1, ws->dp[i]->time());
	snp2("dp[%d].data=%.7g\n", i+1, ws->dp[i]->data());
    }
    snp("num_dw=%d\n", ws->num_dw);
    for(int i = 0; i < ws->num_dw; i++) {
	snp2("dw[%d].label=%c\n", i+1, ws->dw[i].label);
	snp2("dw[%d].tbeg=%.15g\n", i+1, ws->dw[i].d1->time());
	snp2("dw[%d].tend=%.15g\n", i+1, ws->dw[i].d2->time());
    }
    snp("refsta=%s\n", ts->refsta());
    snp("segtype=%s\n", ts->segtype());
    snp("xchan=%s\n", ts->xChan());
    snp("ychan=%s\n", ts->yChan());
    snp("zchan=%s\n", ts->zChan());
    snp("xpos=%.15g\n", ws->scaled_x0);
    snp("ypos=%.15g", ws->scaled_y0);
#undef snp
}

static void
printSegment(Waveform *ws, int i, string &msg)
{
    GSegment *seg = ws->segment(i);
    char s[50];

#define snp(a,b) snprintf(s, sizeof(s), a, b); msg.append(s)

    msg.clear();
    snp("calib=%.15g\n", seg->calib());
    snp("calper=%.15g\n", seg->calper());
    snp("initial_calib=%.15g\n", seg->initialCalib());
    snp("initial_calper=%.15g\n", seg->initialCalper());
    snp("length=%d\n", seg->length());
    snp("samprate=%.15g\n", 1./seg->tdel());
    snp("tbeg=%.15g\n", seg->tbeg());
    snp("tend=%.15g\n", seg->tend());
#undef snp
}

static void
printTable(CssTableClass *o, string &msg)
{
    char s[50], t[50], *n, *member_address;
    double d;
    int num_members = o->getNumMembers();
    CssClassDescription *des = o->description();

    msg.clear();

#define snp(a,b,c) snprintf(s,sizeof(s), a, b, c); msg.append(s)

    // special case: get associated phase
    if(o->nameIs(cssArrival))
    {
	CssArrivalClass *a = (CssArrivalClass *)o;
	snp("%s=%s\n", "phase", a->phase);
    }

    for(int i = 0; i < num_members; i++)
    {
	member_address = (char *)o + des[i].offset;
	n = des[i].name;
	switch(des[i].type)
	{
	    case CSS_STRING:
		snp("%s=%s\n", n, member_address);
		break;
	    case CSS_QUARK:
		snp("%s=%s\n", n, quarkToString(*(int *)member_address));
		break;
	    case CSS_DATE:
	    case CSS_LDDATE:
		snp("%s=%s\n", n, timeDateString((DateTime *)member_address));
		break;
	    case CSS_LONG:
	    case CSS_JDATE:
		snp("%s=%ld\n", n, *(long *)member_address);
		break;
	    case CSS_INT:
		snp("%s=%d\n", n, *(int *)member_address);
		break;
	    case CSS_DOUBLE:
		snp("%s=%.15g\n", n, *(double *)member_address);
		break;
	    case CSS_FLOAT:
		snp("%s=%.7f\n", n, *(float *)member_address);
		break;
	    case CSS_TIME:
		d = *(double *)member_address;
		timeEpochToString(d, t, sizeof(t)-1, YMONDHMS);
		snp("%s=%s\n", n, t);
		break;
        }
    }
    int m = (int)msg.length();
    if(m > 0) msg.erase(m-1); // the trailing '\n'
#undef snp
}

ParseVar Parser::waveFindIndices(const char *name, string &value)
{
    int i, j, n, num_waveforms, num_col;
    ostringstream os;
    char s[50];
    char *member[100], *values[100];
    gvector<Waveform *> wvec;
    GTimeSeries *ts;

    if( !(num_waveforms = getWaveforms(wvec)) ) {
	value.assign("0");
	return STRING_RETURNED;
    }
	
    j = 1;
    for(i = 0; i < 100; i++) {
	snprintf(s, sizeof(s), "arg%d", j++);
	if( !(member[i] = stringGetArg(name, s)) ) break;

	snprintf(s, sizeof(s), "arg%d", j++);
	if( !(values[i] = stringGetArg(name, s)) ) {
	    value.assign(string("find_indices: missing value for ")+member[i]);
	    free(member[i]);
	    for(j = 0; j < i; j++) {
		free(member[j]);
		free(values[j]);
	    }
	    return VARIABLE_ERROR;
	}
    }
    num_col = i;

    if(num_col == 0) {
	value.assign("find_indices: missing arguments");
	return VARIABLE_ERROR;
    }

    value.clear();

    for(i = 0; i < num_waveforms; i++)
    {
	ts = wvec[i]->ts;
	
 	bool ok = true;
	for(j = 0; j < num_col && ok; j++) 
	{
	    if(!strcasecmp(member[j], "sta")) {
		if(strcasecmp(values[j], ts->sta())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "chan")) {
		if(strcasecmp(values[j], ts->chan())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "net")) {
		if(strcasecmp(values[j], ts->net())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "refsta")) {
		if(strcasecmp(values[j], ts->refsta())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "xchan")) {
		if(strcasecmp(values[j], ts->xChan())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "ychan")) {
		if(strcasecmp(values[j], ts->yChan())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "zchan")) {
		if(strcasecmp(values[j], ts->zChan())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "instype")) {
		if(strcasecmp(values[j], ts->instype())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "segtype")) {
		if(strcasecmp(values[j], ts->segtype())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "datatype")) {
		if(strcasecmp(values[j], ts->datatype())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "clip")) {
		if(strcasecmp(values[j], ts->clip())) ok = false;
	    }
	    else if(!strcasecmp(member[j], "chanid")) {
		snprintf(s, sizeof(s), "%d", ts->chanid());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "lat")) {
		snprintf(s, sizeof(s), "%.15g", ts->lat());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "lon")) {
		snprintf(s, sizeof(s), "%.15g", ts->lon());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "elev")) {
		snprintf(s, sizeof(s), "%.15g", ts->elev());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "dnorth")) {
		snprintf(s, sizeof(s), "%.15g", ts->dnorth());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "deast")) {
		snprintf(s, sizeof(s), "%.15g", ts->deast());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "hang")) {
		snprintf(s, sizeof(s), "%.15g", ts->hang());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "vang")) {
		snprintf(s, sizeof(s), "%.15g", ts->vang());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "jdate")) {
		snprintf(s, sizeof(s), "%ld", ts->jdate());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "tbeg")) {
		snprintf(s, sizeof(s), "%.15g", ts->tbeg());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "tend")) {
		snprintf(s, sizeof(s), "%.15g", ts->tend());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "mean")) {
		snprintf(s, sizeof(s), "%.15g", ts->mean());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "duration")) {
		snprintf(s, sizeof(s), "%.15g", ts->duration());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "nsegs")) {
		snprintf(s, sizeof(s), "%d", ts->size());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "length")) {
		snprintf(s, sizeof(s), "%d", ts->length());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "samprate")) {
		snprintf(s, sizeof(s), "%.15g", 1./ts->segment(0)->tdel());
		if(strcmp(values[j], s)) ok = false;
	    }
	    else if(!strcasecmp(member[j], "calib")) {
		snprintf(s, sizeof(s), "%.15g", ts->segment(0)->calib());
		if(strcmp(values[j], s)) ok = false;
	    }
	}
	if(ok) {
	    os << i+1 << ",";
	}
    }
    value.assign(os.str());
    n = (int)value.length();
    if(n > 0 && value[n-1] == ',') value.erase(n-1);

    for(i = 0; i < num_col; i++) {
	free(member[i]);
	free(values[i]);
    }

    return STRING_RETURNED;
}
