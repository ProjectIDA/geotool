#include "config.h"
#include <string.h>

#include "libgdb.h"
#include "libstring.h"

#ifdef HAVE_LIBODBC

long
ODBCGetChanid(SQLHDBC hdbc, const string &sitechanTable, const string &sta,
		const string &chan)
{
	char query[200], s[20], c[20];
	long chanid = -1;
	gvector<CssTableClass *> v;

	stringcpy(s, sta.c_str(), 20);
	stringcpy(c, chan.c_str(), 20);
	stringToUpper(s);
	stringToUpper(c);

	snprintf(query, 200,
		"select * from %s where upper(sta)='%s' and upper(chan)='%s'",
		sitechanTable.c_str(), s, c);

	if(ODBCQueryTable(hdbc, query, "sitechan", v)) {
	    logErrorMsg(LOG_WARNING, ODBCErrMsg());
	}
	if(v.size() > 0) {
	    chanid = ((CssSitechanClass *)v.at(0))->chanid;
	}
	return chanid;
}
#endif /* HAVE_LIBODBC */
