/** \file SeedInput.cpp
 *  \brief Defines a class for reading Seed objects from a C++ istream.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sstream>
#include <algorithm>
#include "seed/SeedInput.h"
#include "seed/DataRecord.h"
#include "seed/ByteOrder.h"
using namespace std;

static bool sortRecords(const DataRecord *a, const DataRecord *b);

/**
 *  A SeedInput is an input stream for reading Seed objects.
 *
 *@see Seed
 */

/** Read a String from the next numBytes of input.
 *  Intermixing calls to readString with readSeed gives unpredictable
 *  results.
 */
string SeedInput::readString(int num_bytes)
{
    char *b = new char[num_bytes];
    string s;

    try {
	readBytes(b, num_bytes);
    }
    catch(...) {
	delete [] b;
	throw;
    }
    s.append(b, num_bytes);
    delete [] b;
    return s;
}

/** Read bytes directly from the stream.
 *  Intermixing calls to readBytes with readSeed gives unpredictable
 *  results.
 */
void SeedInput::readBytes(char *bytes, int len)
{
    if(logical_offset + len > lreclen) {
	int n = lreclen - logical_offset;
	in->read(bytes, n);
	if(!in->good()) throw IOException();

	readVolumeHeader();
	len -= n;
	while(len > lreclen - 8) {
	    in->read(bytes+n, lreclen-8);
	    if(!in->good()) throw IOException();
	    n += (lreclen-8);
	    readVolumeHeader();
	    len -= (lreclen-8);
	}
	if(len > 0) {
	    in->read(bytes+n, len);
	    if(!in->good()) throw IOException();
	    logical_offset += len;
	}
    }
    else {
	if(logical_offset == 0) { // read next logical record control header
	    readVolumeHeader();
	}
	in->read(bytes, len);
	if(!in->good()) throw IOException();
	logical_offset += len;
    }
}

void SeedInput::readVolumeHeader()
{
    char c[8];
    string s;

    in->read(c, 8); // read 8 byte volume control header
    if(!in->good()) throw IOException();
    logical_offset = 8;
    record_index++;

    try {
	s = Seed::trim(string(c, 6));
	record_seqno = Seed::parseInt(s);
    }
    catch ( FormatException &e ) {
	ostringstream out;
	out << "bad record sequence number for record " << record_index;
	if(bits & SEQNOBIT) {
	    throw SeqnoException(out.str());
	}
	cerr << "Warning: " << out.str() << endl;
    }
    record_type = c[6];
    record_continuation = c[7];

    if(record_type == ' ') {
	throw SkipException("record type");
    }
    else if( ctrl_type.find(record_type) != string::npos ) {
	reading_data = false;
    }
    else if( data_quality.find(record_type) != string::npos ) {
	reading_data = true;
    }
    else if(!reading_data && record_continuation == '*') {
	// ok
    }
    else {
	ostringstream out;
	out << "unknown header type (byte 7): '" << record_type << "'";
	if(bits & HDRBIT) {
	    throw HdrException(out.str());
	}
	cerr << "Warning: " << out.str() << endl;
    }
}

/** Read the next Seed object. */
Seed * SeedInput::readSeed()
{
    Seed *o = NULL;
    DataRecord *dr;

    // skip DataRecord's with nsamples <= 0 or samprate <= 0.
    if( ! (mode & RAWBIT) ) {
	while((o = getNextObject()) && (dr = o->getDataRecord())
		&& (dr->header.nsamples <= 0 || dr->samprate <= 0.)) delete o;
    }

    if( !o ) {
	return o;
    }
    else if( (dr = o->getDataRecord()) )
    {
	if( mode & RAWBIT ) {
	    return dr;
	}
	else {
	    return getSeedData(dr);
	}
    }
    return o;
}

Seed * SeedInput::getSeedData(DataRecord *dr)
{
    DataHeader *h;
    SeedData *sd;
    Seed *o;
    vector<DataRecord *> records;
    double tnext, diff;
    int i;

    h = &dr->header;
    records.push_back(dr);

    // read all data records for the same station and network.
    try {
	while((o = getNextObject()) && (dr = o->getDataRecord())
		&& !h->station.compare(dr->header.station)
		&& !h->network.compare(dr->header.network))
	{
	    if(dr->header.nsamples > 0) {
		records.push_back(dr);
	    }
	    else {
		delete dr;
	    }
	}
    }
    catch ( IOException &e ) {
    }
    catch(...) {
	for(i = 0; i < (int)records.size(); i++) delete records[i];
	throw;
    }

    // sort by channel, location, samprate, and time
    sort(records.begin(), records.end(), sortRecords);

    // for each channel, collect data records that are continuous
    sd = new SeedData(records[0]);
    getCalib(sd);
    next.push_back(sd);

    for(i = 1; i < (int)records.size(); i++) {
	dr = records[i-1];
	tnext = dr->header.startTime() + dr->header.nsamples/dr->samprate;
	diff = fabs(records[i]->header.startTime() - tnext);

	if(!dr->header.channel.compare(records[i]->header.channel)
	    && !dr->header.location.compare(records[i]->header.location)
	    && dr->header.dhqual == records[i]->header.dhqual
	    && (diff < .5/dr->samprate ||
			diff <= dr->header.nsamples*dr->clock_drift))
	{
	    // the SeedData object holds continuous data for one channel
	    sd->records.push_back(*records[i]);
	}
	else {
	    sd = new SeedData(records[i]);
	    getCalib(sd);
	    next.push_back(sd);
	}
    }
    // the contents of all DataRecord objects have been copied to SeedData
    // objects, so can delete all the DataRecord's
    for(i = 0; i < (int)records.size(); i++) delete records[i];

    if(o) next.push_back(o); // the next DataRecord for a different sta/chan

    o = next.front();
    next.erase(next.begin());
    return o;
}

static bool sortRecords(const DataRecord *a, const DataRecord *b)
{
    int ret;
    double atime, btime;

    if( (ret = a->header.channel.compare(b->header.channel)) ) return ret < 0;
    if( (ret = a->header.location.compare(b->header.location)) ) return ret < 0;
    if( a->samprate != b->samprate) return a->samprate < b->samprate;

    atime = a->header.start_time.epoch();
    btime = b->header.start_time.epoch();
    return atime < btime;
}

Seed * SeedInput::getNextObject()
{
    if((int)next.size() > 0) {
	Seed *s = next.front();
	next.erase(next.begin());
	return s;
    }
    state = 0;

    for(;;) {
	try {
	    return readSeedObject();
	}
	catch ( LenException &e ) {
	    state |= LENBIT;
	    if(bits & LENBIT) throw;
	    skipRec();
	}
	catch ( HdrException &e ) {
	    state |= HDRBIT;
	    if(bits & HDRBIT) throw;
	    skipRec();
	}
	catch ( SkipException &e ) {
	    skipRec();
	}
	catch ( IOException &e ) {
	    if( !strcmp(e.what(), "EOF") ) return NULL;
	    throw;
	}
	catch ( FormatException &e ) {
	    state |= FMTBIT;
	    if(bits & FMTBIT) throw;
	    // skip to next blockette
	}
	catch(ios_base::failure) {
	    if(in->eof()) return NULL;
	    throw;
	}
    }
    return NULL;
}

Seed *SeedInput::readSeedObject()
{
    int i, blockette_length;
    string blockette_type, blockette_fields;
    string s, name;
    Blockette5 *b5;
    Blockette8 *b8;
    Blockette10 *b10;
    Blockette50 *b50;
    Blockette60 *b60;
    Seed *blockette;

    /* if there are less than 7 bytes remaining in the volume,
     * skip to the next logical volume
     */
    if(lreclen - logical_offset < 7) {
	throw SkipException("EOR");
    }

    try {
	s = readString(3);
    }
    catch(IOException &e) {
	throw IOException("EOF");
    }

    if( !reading_data && !s.compare("   ") ) {
	throw SkipException("blockette type");
    }

    if( !reading_data )
    {
	// get next blockette
	blockette_type = Seed::trim(s);
	try {
	    blockette_length = Seed::parseInt(Seed::trim(readString(4)));
	}
	catch(FormatException e) {
	    throw LenException(blockette_type + ":bad length");
	}
	blockette_fields = readString(blockette_length-7);

	blockette = NULL;
	for(i = 0; i < (int)blockette_type.length()
		&& blockette_type[i] == '0'; i++);
	if(i < (int)blockette_type.length()) {
	    name = "Blockette" + blockette_type.substr(i);
	    blockette = Seed::createBlockette(name);
	}

	if( !blockette ) {
	    cerr << "Warning: Unknown blockette type:" << s << endl;
	    blockette = new UnknownBlockette(blockette_type, blockette_fields);
	}
	else {
	    try {
		blockette->load(blockette_fields);
	    }
	    catch(...) {
		delete blockette;
		throw;
	    }
	}

	/* if the blockette contains the logical_record_length (types 5, 8, 10),
	 * then set lreclen
	 */
	if( (b5 = blockette->getBlockette5()) ) {
	    lreclen = 2;
	    for(i = 1; i < b5->logical_record_length; i++) lreclen *= 2;
	}
	else if( (b8 = blockette->getBlockette8()) ) {
	    lreclen = 2;
	    for(i = 1; i < b8->logical_record_length; i++) lreclen *= 2;
	}
	else if( (b10 = blockette->getBlockette10()) ) {
	    lreclen = 2;
	    for(i = 1; i < b10->logical_record_length; i++) lreclen *= 2;
	}
	else if( (b50 = blockette->getBlockette50()) ) {
	    // save station blockettes
	    for(i = 0; i < (int)stations.size(); i++) {
		if( !stations[i]->b50.station.compare(b50->station) &&
		    !stations[i]->b50.network.compare(b50->network))
		{
		    // remove station with same name
		    delete stations[i];
		    stations.erase(stations.begin()+i);
		    break;
		}
	    }
	    stations.push_back(new Station(b50));
	}
	else if(blockette->getBlockette51() || blockette->getBlockette52() ||
		blockette->getBlockette53() || blockette->getBlockette54() ||
		blockette->getBlockette55() || blockette->getBlockette56() ||
		blockette->getBlockette57() || blockette->getBlockette58() ||
		blockette->getBlockette59() || blockette->getBlockette61() ||
		blockette->getBlockette62())
	{
	    if((int)stations.size() == 0) {
		cerr << "Blockette" << blockette->getType()
			<< " found before Blockette50" << endl;
	    }
	    stations.back()->add(blockette);
	}
	else if( (b60 = blockette->getBlockette60()) ) {
	    if((int)stations.size() == 0) {
		cerr << "Blockette" << blockette->getType()
			<< " found before Blockette50" << endl;
	    }
	    stations.back()->add(convertBlockette60(b60));
	}
	else if(blockette->getDictionaryBlockette())
	{
	    dictionary.add(blockette->getDictionaryBlockette());
	}
	else if(blockette->getBlockette5() || blockette->getBlockette10() ||
		blockette->getBlockette11() || blockette->getBlockette12())
	{
	    dictionary.clear();
	    stations.clear();
	}
	return blockette;
    }
    else {
	return readDataRecord(s);
    }
}

DataRecord * SeedInput::readDataRecord(const string &s)
{
    int i, j, n, next_blockette, offset, logical_pos, seqno;
    int data_record_len = lreclen;
    char b[4], c[4], rtype;
    string sta, net;
    string bytes, word_order, short_order, blockette_type, name;
    DataRecord *dr;
    Blockette1000 *b1000 = NULL;
    Blockette100 *b100;
    UWORD u;

    if(logical_offset > 11) {
	// not the first data record in the logical volume.
	// Read four more bytes to get to the 7'th byte of the header
	bytes = s + readString(4);
	rtype = bytes[6];

	if(rtype == ' ') {
	    throw SkipException("data record type");
	}
	try {
	    seqno = Seed::parseInt(Seed::trim(bytes.substr(0, 6)));
	}
	catch ( FormatException &e ) {
	    ostringstream out;
	    out << "bad record sequence number for data record";
	    if(bits & SEQNOBIT) {
		throw SeqnoException(out.str());
	    }
	    cerr << "Warning: " << out.str() << endl;
	    seqno = -1;
	}
	readBytes(b, 1); // skip byte 8
	bytes = readString(40);
    }
    else {
	// 8 bytes of header has already been read plus 3 more bytes
	bytes = s + readString(37);
	rtype = record_type;
	seqno = record_seqno;
    }

    // get station and network name from data header
    sta = Seed::trim(bytes.substr(0, 5));
    net = Seed::trim(bytes.substr(10, 2));
    // get word order from b50 blockette. If b50 not found, assume big-endian
    getWordOrder(sta, net, &word_order, &short_order);

    dr = new DataRecord(bytes, word_order, short_order);

    dr->header.seqno = seqno;
    dr->header.dhqual = rtype;
    dr->samprate = dr->header.sample_rate;

    logical_pos = logical_offset - 48;

    try {
	dr->record_offset = (int)in->tellg() - 48;
	dr->data_file_offset = dr->record_offset + dr->header.offset;

	// skip to the beginning of the first data blockette
	offset = dr->header.boffset - 48;
	for(i = 0; i < offset; i++) readBytes(b, 1);

	// read data blockettes
	for(i = 0; i < dr->header.num; i++) {
	    ostringstream out;

	    readBytes(b, 4);

	    u.a[dr->so[0]] = b[0]; u.a[dr->so[1]] = b[1];
	    if(u.s == 59395) { // this is 1000 with the wrong byte order
		// this can happen if this is a mini-seed (no b50) and the
		// data header was not written in big-endian order
		// reverse the current header byte order
		c[0] = word_order[3]; c[1] = word_order[2];
		c[2] = word_order[1]; c[3] = word_order[0];
		word_order.assign(c, 4);
		c[0] = short_order[1];
		c[1] = short_order[0];
		short_order.assign(c, 2);
		// reload the header with the corrected word order
		dr->resetWordOrder(bytes, word_order, short_order);
		blockette_type.assign("1000");
	    }
	    else {
		out << u.s;
		blockette_type = out.str();
	    }

	    u.a[dr->so[0]] = b[2]; u.a[dr->so[1]] = b[3];
	    next_blockette = (int)u.s;

	    name = "Blockette" + blockette_type;
	    DataBlockette *db = Seed::createDataBlockette(name);
	    if( !db ) {
		cerr << "Unknown data blockette type: '" << blockette_type
			<< "'" << endl;
	    }
	    else {
		n = db->length() - 4;
		bytes = readString(n);
		try {
		    db->loadDB(bytes, dr->wo, dr->so);
		}
		catch(LenException e) {
		    if(bits & LENBIT) {
			delete db;
			throw;
		    }
		}
		dr->blockettes.push_back(db);

		if( db->getBlockette1000() ) {
		    b1000 = db->getBlockette1000();
		    data_record_len = 2;
		    for(j = 1; j < b1000->lreclen; j++) data_record_len *= 2;
		    dr->reclen = data_record_len;
		    if(b1000->word_order) { // big endian
			dr->word_order.assign("3210");
			dr->short_order.assign("10");
		    }
		    else {
			dr->word_order.assign("0123");
			dr->short_order.assign("01");
		    }
		    dr->swapOrder();
		    dr->format = b1000->format;
		}
		else if( (b100 = db->getBlockette100()) ) {
		    dr->samprate = b100->sample_rate;
		}
	    }
	    if(next_blockette > 0) {
		// skip to the next blockette
		n = next_blockette - logical_offset;
		for(j = 0; j < n; j++) readBytes(b, 1);
	    }
	}
	dr->header.num = (int)dr->blockettes.size();
	dr->data_length = data_record_len - dr->header.offset;

	if( mode & DATABIT ) {
	    n = logical_pos + dr->header.offset - logical_offset;
	    for(j = 0; j < n; j++) readBytes(b, 1);

	    dr->data = readString(dr->data_length);
	}
	else {
	    n = logical_pos + dr->header.offset - logical_offset;
	    n += dr->data_length;
	    in->seekg(n, ios::cur);
	    logical_offset += n;
	}
	getFormat(dr, b1000);
    }
    catch(...) {
	delete dr;
	throw;
    }

    return dr;
}

void SeedInput::getWordOrder(string &sta, string &net, string *word_order,
			string *short_order)
{
    int i;
    for(i = 0; i < (int)stations.size(); i++) {
	if( !sta.compare(stations[i]->b50.station)
	 && !net.compare(stations[i]->b50.network) ) break;
    }
    if(i < (int)stations.size()) {
	if(stations[i]->b50.word_order.length() == 4 &&
	    stations[i]->b50.word_order.find('0') != string::npos &&
	    stations[i]->b50.word_order.find('1') != string::npos &&
	    stations[i]->b50.word_order.find('2') != string::npos &&
	    stations[i]->b50.word_order.find('3') != string::npos)
	{
	    *word_order = stations[i]->b50.word_order;
	}
	else {
	    cerr << "Warning: invalid blockette50 32 bit word order" << endl;
	    *word_order = string("3210");
	}

	if(stations[i]->b50.short_order.length() == 2 &&
	    stations[i]->b50.short_order.find('0') != string::npos &&
	    stations[i]->b50.short_order.find('1') != string::npos)
	{
	    *short_order = stations[i]->b50.short_order;
	}
	else {
	    cerr << "Warning: invalid blockette50 16 bit word order" << endl;
	    *short_order = string("10");
	}
    }
    else {
	// assume big-endian word order
	*word_order = string("3210");
	*short_order = string("10");
    }
}

void SeedInput::getFormat(DataRecord *dr, Blockette1000 *b1000)
{
    DataHeader *dh = &dr->header;
    Channel *channel = NULL;
    int i, j;

    for(i = 0; i < (int)stations.size() && !channel; i++)
	if( !dh->network.compare(stations[i]->b50.network) &&
	    !dh->station.compare(stations[i]->b50.station))
    {
	for(j = 0; j < (int)stations[i]->channels.size(); j++)
	    if(!dh->channel.compare(stations[i]->channels[j]->b52.channel) &&
	       !dh->location.compare(stations[i]->channels[j]->b52.location))
	{
	    channel = stations[i]->channels[j];
	    break;
	}
    }
    if( channel && channel->b52.clock_drift > 0. ) {
	dr->clock_drift = channel->b52.clock_drift;
    }
    else {
	dr->clock_drift = 0.0001;
    }

    // If have blockette1000, then get data format from it.  Otherwise, use B30
    if( b1000 ) {
	dr->format = b1000->format;
    }
    else {
	Blockette30 *b30 = NULL;
	if(channel) {
	    b30 = dictionary.getB30(channel->b52.format_code);
	}
	if( b30 ) {
	    char *s = strdup(b30->name.c_str());
	    for(i = 0; i < (int)strlen(s); i++) s[i] = toupper((int)s[i]);

	    if(strstr(s, "16-BIT")) {
		dr->format = 1;
	    }
	    else if(strstr(s, "24-BIT")) {
		dr->format = 2;
	    }
	    else if(strstr(s, "32-BI")) {
		dr->format = 3;
	    }
	    else if(strstr(s, "SUN I")) {
		dr->format = 4;
	    }
	    else if(strstr(s, "STEIM1") || strstr(s, "STEIM-1") ||
		    strstr(s, "STEIM 1"))
	    {
		dr->format = 10;
	    }
	    else if(strstr(s, "STEIM2") || strstr(s, "STEIM-2") ||
		    strstr(s, "STEIM 2"))
	    {
		dr->format = 11;
	    }
	    else if(strstr(s, "STEIM INT"))
	    {
		dr->format = 11;
	    }
	    else {
		cerr << "Cannot determine encoding format: "
			<< b30->name.c_str() << endl;
	    }
	    free(s);
	    // need more choices here.
	}
	else {
	    throw HdrException("Data header with no B1000 for "
		    + dh->network+"/"+dh->station+"/"+dh->channel
		    +" encountered before corresponding B30 blockette.");
	}
    }
}

void SeedInput::getCalib(SeedData *sd)
{
    Blockette34 *b34 = NULL;
    Blockette48 *b48 = NULL;
    Blockette58 *b58 = NULL;
    Blockette60 *b60 = NULL;
    Channel *channel = NULL;
    DataHeader *dh;
    double displacement_factor;
    int i;

    if((int)sd->records.size() <= 0) return;
    dh = &sd->records[0].header;

    sd->calib = 0.0;
    sd->calper = 0.0;

    for(i = 0; i < (int)stations.size() && !channel; i++)
	if( !dh->network.compare(stations[i]->b50.network) &&
	    !dh->station.compare(stations[i]->b50.station))
    {
	for(int j = 0; j < (int)stations[i]->channels.size(); j++) {
	    if( !dh->channel.compare(stations[i]->channels[j]->b52.channel) &&
		!dh->location.compare(stations[i]->channels[j]->b52.location))
	    {
		channel = stations[i]->channels[j];
		break;
	    }
	}
    }
    if( !channel ) return;

    sd->channel = new Channel(*channel);

    for(i = 0; i < (int)channel->response.size(); i++)
	if(channel->response[i]->getBlockette58() &&
	   channel->response[i]->getBlockette58()->stage == 0)
    {
	b58 = channel->response[i]->getBlockette58();
	break;
    }

    displacement_factor = 1.0;

    if( b58 )
    {
	if( (b34 = dictionary.getB34(channel->b52.signal_units)) )
	{
	    char *s = strdup(b34->description.c_str());
	    for(i = 0; i < (int)strlen(s); i++) s[i] = toupper((int)s[i]);
	    if( strstr(s, "VEL") ) {
		displacement_factor = 2.*M_PI*b58->frequency;
	    }
	    else if( strstr(s, "ACCEL") ) {
		displacement_factor =4.*M_PI*M_PI*b58->frequency*b58->frequency;
	    }
	    free(s);
	}
    }

    for(i = 0; i < (int)channel->response.size(); i++) {
	if( (b58 = channel->response[i]->getBlockette58()) && b58->stage == 0) {
	    sd->calib = 1.0e+9/(b58->sensitivity*displacement_factor);
	    sd->calper = 1.0/b58->frequency;
	    break;
	}
	else if( (b60 = channel->response[i]->getBlockette60()) ) {
	    for(int j = 0; j < (int)b60->response.size() && !b48; j++) {
		if(b60->response[j].stage == 0) {
		    for(int k = 0; k < (int)b60->response[j].code.size(); k++){
			b48 = dictionary.getB48(b60->response[j].code[k]);
			if( b48 ) break;
		    }
		}
	    }
	    if(b48) {
		sd->calib = 1.0e+9/(b48->sensitivity*displacement_factor);
		sd->calper = 1.0/b48->frequency;
		break;
	    }
	}
    }
}

/** Convert blockettes referenced in b60 (43 - 48 and 41) to (53 - 58 and 61)
 */
Seed * SeedInput::convertBlockette60(Blockette60 *b60)
{
    int i, j;
    Blockette41 *b41;
    Blockette43 *b43;
    Blockette44 *b44;
    Blockette45 *b45;
    Blockette46 *b46;
    Blockette47 *b47;
    Blockette48 *b48;

    for(i = 0; i < (int)b60->response.size(); i++)
    {
	for(j = 0; j < (int)b60->response[i].code.size(); j++) {
	    if( (b41 = dictionary.getB41(b60->response[i].code[j])) ) {
		return new Blockette61(b60->response[i].stage, *b41);
	    }
	    else if( (b43 = dictionary.getB43(b60->response[i].code[j])) ) {
		return new Blockette53(b60->response[i].stage, *b43);
	    }
	    else if( (b44 = dictionary.getB44(b60->response[i].code[j])) ) {
		return new Blockette54(b60->response[i].stage, *b44);
	    }
	    else if( (b45 = dictionary.getB45(b60->response[i].code[j])) ) {
		return new Blockette55(b60->response[i].stage, *b45);
	    }
	    else if( (b46 = dictionary.getB46(b60->response[i].code[j])) ) {
		return new Blockette56(b60->response[i].stage, *b46);
	    }
	    else if( (b47 = dictionary.getB47(b60->response[i].code[j])) ) {
		return new Blockette57(b60->response[i].stage, *b47);
	    }
	    else if( (b48 = dictionary.getB48(b60->response[i].code[j])) ) {
		return new Blockette58(b60->response[i].stage, *b48);
	    }
	    else {
		cerr << "Cannot find dictionary blockette b60.lookup_code: "
			<< b60->response[i].code[j];
	    }
	}
    }
    return NULL;
}
