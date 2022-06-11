/** \file GSourceInfo.cpp
 *  \brief Defines class GSourceInfo.
 *  \author Ivan Henson
 */
#include "config.h"
#include <iostream>
#include <sys/param.h>

using namespace std;

#include "gobject++/GSourceInfo.h"
#include "gobject++/CssTableClass.h"

/** Constructor. All strings are initialized to "". The directory_duration is
 *  initialized to 0.
 */
GSourceInfo::GSourceInfo(void) :
    dc(stringToQuark("")),
    format(stringToQuark("")),
    data_source(stringToQuark("")),
    user(stringToQuark("")),
    passwd(stringToQuark("")),
    account(stringToQuark("")),
    directory_structure(stringToQuark("")),
    directory_duration(0.),
    table_name(stringToQuark("")),
    dir(stringToQuark("")),
    prefix(stringToQuark(""))
{
}

/** Constructor with a file path argument.
 */
void GSourceInfo::setSource(const string &path)
{
    char directory[MAXPATHLEN+1];

    dc = stringToQuark(path);

    memset(directory, 0, sizeof(directory));
    strncpy(directory, path.c_str(), MAXPATHLEN);
    int i;
    for(i = (int)strlen(directory)-1; i >= 0 && directory[i] != '/'; i--);
    if(i == 0 && directory[i] == '/') {
	dir = stringToQuark("/");
	prefix = stringToQuark(directory+1);
    }
    else if(i > 0) {
	directory[i] = '\0';
	dir = stringToQuark(directory);
	prefix = stringToQuark(directory+i+1);
    }
    else {
	dir = stringToQuark("");
	prefix = stringToQuark(path);
    }

    format = stringToQuark("");
    data_source = stringToQuark("");
    user = stringToQuark("");
    passwd = stringToQuark("");
    account = stringToQuark("");
    directory_structure = stringToQuark("");
    directory_duration = 0.;
    table_name = stringToQuark("");
}

/** Set the source information from a CssTableClass. Data source information is
 *  obtained from the CssTableClass object.
 *  @param[in] table a CssTableClass with source information.
 */
void GSourceInfo::setSource(CssTableClass *table)
{
    dc = table->getDC();
    format = table->getFormat();
    dir = table->getDir();
    prefix = table->getPrefix();
    table->getSource(&data_source, &user, &passwd);
    table->getAccount(&account, &table_name);
    table->getDirectoryStructure(&directory_structure, &directory_duration);
}

/** Set the source information from another GSourceInfo object.
 *  @param[in] s
 */
void GSourceInfo::setSource(GSourceInfo &s)
{
    dc = s.dc;
    format = s.format;
    data_source = s.data_source;
    user = s.user;
    passwd = s.passwd;
    account = s.account;
    directory_structure = s.directory_structure;
    directory_duration = s.directory_duration;
    table_name = s.table_name;
    dir = s.dir;
    prefix = s.prefix;
}

/** Copy this source information to a CssTableClass. Data source information is
 *  copied from this GSourceInfo instance to the CssTableClass object.
 *  @param[in,out] table a CssTableClass object.
 */
void GSourceInfo::copySource(CssTableClass *table)
{
//    table->setIds(dc, table->getID());
    table->setDC(dc);
    table->setFormat(format);
    table->setDir(dir);
    table->setPrefix(prefix);
    table->setSource(data_source, user, passwd);
    table->setAccount(account, table_name);
    table->setDirectoryStructure(directory_structure, directory_duration);
}
