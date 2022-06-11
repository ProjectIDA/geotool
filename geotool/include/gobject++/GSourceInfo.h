#ifndef _GSOURCE_INFO_H
#define _GSOURCE_INFO_H

#include "gobject++/CssTableClass.h"
extern "C" {
#include "libstring.h"
}

/** A class that holds data source information.
 *  @ingroup libgobject
 */
class GSourceInfo
{
    public:
	GSourceInfo(void);
	~GSourceInfo(void) {}

	void setSource(const string &path);
	void setSource(CssTableClass *a);
	void setSource(GSourceInfo &s);
	void copySource(CssTableClass *a);

	/** Get the ODBC data source
	 *  @returns the ODBC data source string.
	 */
	const char *getODBCDataSource() { return quarkToString(data_source); }
	/** Get the user.
	 *  @returns the user character string.
	 */
	const char *getUser() { return quarkToString(user); }
	/** Get the parameter root for an FFDB source.
	 *  @returns the parameter root character string.
	 */
	const char *getParamRoot() { return quarkToString(user); }
	/** Get the segment root for an FFDB source.
	 *  @returns the segment root character string.
	 */
	const char *getSegRoot() { return quarkToString(passwd); }
	/** Get the password for an ODBC source.
	 *  @returns the password character string.
	 */
	const char *getPassword() { return quarkToString(passwd); }
	/** Get the account for an ODBC source.
	 *  @returns the account character string.
	 */
	const char *getAccount() { return quarkToString(account); }
	/** Get the directory.
	 *  @returns the directory character string.
	 */
	const char *getDir() { return quarkToString(dir); }
	/** Get the directory structure string.
	 *  @returns the directory structure string.
	 */
	const char *getDirStruct() { return quarkToString(directory_structure);}
	/** Get the prefix.
	 *  @returns the prefix character string.
	 */
	const char *getPrefix() { return quarkToString(prefix); }

	int	dc;	     //!< the table dc.
	int	format;	     //!< the format quark
	int	data_source; //!< the data source quark.
	int	user;	     //!< the user quark or parameter root quark
	int	passwd;      //!< the password quark or segment root quark
	int	account;     //!< the account quark
	int	directory_structure; //!< the directory structure quark
	double	directory_duration; //!< the directory time duration
	int	table_name;   //!< the table name quark.
	int	dir;          //!< the directory quark.
	int	prefix;       //!< the prefix quark.

    protected:
};

#endif

