#ifndef _RESPONSE_FILE_H
#define _RESPONSE_FILE_H

#include "gobject++/CssTables.h"
#include "gobject++/cvector.h"
#include "Response.h"

/** A class for reading instrument response files. This class reads three types
 *  of instrument responses, FAP (frequency, amplitude, and phase), PAZ (poles
 *  and zeroes), and FIR (finite impulse response). It reads two different
 *  file formats, the standard CSS response file and the GSE2.0 formatted file.
 *  Both of these response file types can contain one or more of the three
 *  types of responses. Response objects are created for each response found
 *  in the file.
 *  @ingroup libgresp
 */
class ResponseFile : public Gobject
{
    public:
	ResponseFile() {}
	bool ioError(void) { return io_error; }

	static bool getFile(const string &sta, const string &chan, double time,
		double endtime, string &insname, string &filename, int *inid,
		const char **err_msg);
	static bool getFile(const string &sta, const string &chan, double time,
		double endtime, const string &dir, const string &prefix,
		string &insname, string &filename, int *inid,
		const char **err_msg);

	static ResponseFile * readFile(const string &filename, bool warn=true);
	static ResponseFile * readFile(CssInstrumentClass *n, bool warn=true);
	static int getInstruments(cvector<CssInstrumentClass> &v);
	static CssInstrumentClass * getInstrument(ResponseFile *rf, int inid);
	static CssInstrumentClass * getInstrument(int inid);
	static CssInstrumentClass * addDFile(const string &file);

	string file; //!< the filename
	gvector<Response *> responses; //!< Response objects

    protected:
	bool io_error;
	ResponseFile(const string &filename);
	ResponseFile(CssInstrumentClass *ins);
	~ResponseFile(void);
	bool open(void);
	bool openCssFile(void);
	bool determineType(char *line, Response *resp, int line_num);
	bool readPAZ(FILE *fp, Response *resp, int *line_num);
	bool readFAP(FILE *fp, Response *resp, int *line_num);
	bool readFIR(FILE *fp, Response *resp, int *line_num);
	bool readGseResponse(void);
	bool readCAL2(char *line, GSE_CAL *cal, int line_num,
		const string &file);
	Response * readPAZ2(FILE *fp, char *line, GSE_CAL *cal,
		int *line_num, const string &file);
	Response * readFAP2(FILE *fp, char *line, GSE_CAL *cal,
		int *line_num, const string &file);
	Response * readFIR2(FILE *fp, char *line, GSE_CAL *cal,
		int *line_num, const string &file);

    private:
	bool openTGFile(void);
	bool read_tgPaz(FILE *fp, Response *resp, int *line_num);
	bool read_tgFap(FILE *fp, Response *resp, int *line_num);
};

#endif
