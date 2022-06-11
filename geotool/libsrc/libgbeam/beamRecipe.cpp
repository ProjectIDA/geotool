/** \file beamRecipe.cpp
 *  \brief Defines class Beam.
 *  \author Ivan Henson
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>

#include "Beam.h"
#include "gobject++/Gobject.h"
#include "motif++/Application.h"
#include "gobject++/DataSource.h"

extern "C" {
#include "libstring.h"
}

static vector<BeamRecipe> origin_recipes;
static vector<BeamRecipe> detection_recipes;

#define file_warn(path) \
    if(errno > 0) \
    { \
	GError::setMessage("Cannot open: %s\n%s", path, strerror(errno)); \
    } \
    else \
    { \
        GError::setMessage("Cannot open: %s", path); \
    }

static int get_non_blank(FILE *fp, char *line, int n);
static bool blankLine(const char *line);
static int readRecipes(const string &recipe_directory,
			const string &recipe_directory2,
			vector<BeamRecipe> &recipes, int type);
static int read_recipes(int type, const string &recipe_directory,
			vector<BeamRecipe> &recipes);
static bool findParTable(FILE *fp, const char *name);
static bool getVariablePositions(FILE *fp, int num, const char **variable,
			int *pos);
static int getRecipes(const string &recipe_directory, const char *file,
		const char *path, vector<BeamRecipe> &recipes);
static void parseLine(char *line, int *pos, BeamRecipe *r);
static bool add_group(const char *path, string &group, vector<BeamSta> &sta);
static int new_group_file(const string &net, const char *path, string &group,
			vector<BeamSta> &sta);
static int read_groups(const string &recipe_directory,
			vector<BeamGroup> &groups);
static bool getGroups(const char *file, const char *dir,
			vector<BeamGroup> &groups);
static int readGroupFile(FILE *fp, const char *dir, const char *path,
			const char *net, vector<BeamGroup> &groups);
static bool add_recipe(const char *recipe_directory, BeamRecipe *recipe);
static int new_recipe_file(const string &recipe_directory, BeamRecipe *recipe,
			bool origin_beam);
static void write_recipe(FILE *fp, int *pos, BeamRecipe *recipe);
static void selectRecipes(vector<BeamRecipe> &r, const string &selected);
static FILE * getIncludePar(const string &recipe_directory, FILE *fp,
			char *path, int path_len);


/** Get an origin beam recipe. The <b>recipe_directory</b> and the <b>net</b>
 *  are used to generate the origin beam recipe filename:
\code
    upper_net = net;
    for(i = 0; upper_net[i] != '\0'; i++) {
	upper_net[i] = toupper(upper_net[i]);
    }
    snprintf(path, sizeof(path), "%s/beam/originbeam/%s-beam.par",
		recipe_directory, upper_net);
\endcode
 *  The recipes are read from the file and the recipe with the input phase is
 *  returned.
 *  @param[in] recipe_directory the recipe directory
 *  @param[in] net the network name
 *  @param[in] phase the recipe phase name.
 *  @param[out] recipe the beam recipe.
 *  @returns  true for success. Returns false if there is an error opening the
 *  file or the recipe for the input phase cannot be found. Use
 *  GError::getMessage to obtain the error message.
 */
bool Beam::getOrigin(const string &recipe_directory, const string &net,
		const string &phase, BeamRecipe *recipe)
{
    string upper_net;
    char path[MAXPATHLEN+1];
    int	 i, num_recipes;
    FILE *fp;
    vector<BeamRecipe> recipes;

    upper_net = net;
    for(i = 0; i < (int)upper_net.length(); i++) {
	upper_net[i] = toupper(upper_net[i]);
    }

    snprintf(path, sizeof(path), "%s/beam/originbeam/%s-beam.par",
		recipe_directory.c_str(), upper_net.c_str());

    if((fp = fopen(path, "r")) == NULL)
    {
	file_warn(path);
	if(fp != NULL) fclose(fp);
	return false;
    }
    if((num_recipes = readFile(recipe_directory, fp, upper_net, path,
		sizeof(path), recipes)) < 0)
    {
	GError::setMessage("No recipes found in %s.", path);
	if(fp != NULL) fclose(fp);
	return false;
    }
    fclose(fp);

    for(i = 0; i < num_recipes && phase.compare(recipes[i].phase); i++);

    if(i == num_recipes) {
	GError::setMessage("Cannot find phase %s in file %s\n",
			phase.c_str(), path);
	return false;
    }

    *recipe = recipes[i];

    return true;
}

/** Read a beam recipe file.
 *  @param[in] recipe_directory the recipe directory
 *  @param[in] fp a FILE pointer positioned at the beginning of the file.
 *  @param[in] net the network name
 *  @param[out] path the recipe filename.
 *  @param[in] path_len the sizeof the character array path[].
 *  @param[in,out] r BeamRecipe structures are appended to r.
 *  @returns  the number of recipes read. Returns -1 if an error occurred.
 *  Use GError::getMessage to obtain the error message.
 *  @throws GERROR_MALLOC_ERRO&
 */
int Beam::readFile(const string &recipe_directory, FILE *fp, const string &net,
		const char *path, int path_len, vector<BeamRecipe> &r)
{
    int pos[14], num_recipe, path_q;
    char line[501];
    const char *variables[] = {
	"name", "type", "rot", "std", "snr", "azi", "slow", "phase",
	"flo", "fhi", "ford", "zp", "ftype", "group"
    };
    FILE *fp2 = NULL;

    if( ! findParTable(fp, "beam-recipe") ) {
	// look for an include file "par="
	fseek(fp, 0, 0);

	char p[MAXPATHLEN+1];
	fp2 = getIncludePar(recipe_directory, fp, p, sizeof(p));
	if(fp2 && findParTable(fp2, "beam-recipe")) {
	    stringcpy((char *)path, p, path_len);
	    fp = fp2;
	}
	else {
	    if(fp2) fclose(fp2);
	    return -1;
	}
    }

    if( !getVariablePositions(fp, 14, variables, pos) || pos[0] == -1
		|| pos[13] == -1)
    {
	if(fp2) fclose(fp2);
	return -1;
    }

    path_q = stringToQuark(path);

    num_recipe = 0;
    while(get_non_blank(fp,line,500) != EOF && strstr(line, "EndTable") == NULL)
    {
	BeamRecipe b;
	b.net = net;
	b.path = path_q;
	b.ford = 3;
	b.zp = 0;

	parseLine(line, pos, &b);

	r.push_back(b);
	num_recipe++;
    }

    if(fp2) fclose(fp2);

    return num_recipe;
}

static FILE *
getIncludePar(const string &recipe_directory, FILE *fp, char *path,int path_len)
{
    char line[501];

    fseek(fp, 0, 0);
    while(get_non_blank(fp, line, 500) != EOF)
    {
	if(!strstr(line, "(sta)") && !strncmp(line, "par=$(PARDIR)/", 14))
	{
	    snprintf(path, path_len, "%s/%s", recipe_directory.c_str(),line+14);
	    return fopen(path, "r");
	}
    }
    return NULL;
}

static void
parseLine(char *line, int *pos, BeamRecipe *r)
{
    int j;
    char *c, *tok, *last;

    tok = line;
    j = 0;
    while((c = strtok_r(tok, "| \t", &last)) != NULL)
    {
	tok = NULL;

	if(j == pos[0]) {
	    r->name.assign(c);
	}
	else if(j == pos[1]) {
	    r->beam_type.assign(c);
	}
	else if(j == pos[2]) {
	    r->rot.assign(c);
	}
	else if(j == pos[3]) {
	    sscanf(c, "%d", &r->std);
	}
	else if(j == pos[4]) {
	    sscanf(c, "%lf", &r->snr);
	}
	else if(j == pos[5]) {
	    sscanf(c, "%lf", &r->azi);
	}
	else if(j == pos[6]) {
	    sscanf(c, "%lf", &r->slow);
	}
	else if(j == pos[7]) {
	    r->phase.assign(c);
	}
	else if(j == pos[8]) {
	    sscanf(c, "%lf", &r->flo);
	}
	else if(j == pos[9]) {
	    sscanf(c, "%lf", &r->fhi);
	}
	else if(j == pos[10]) {
	    sscanf(c, "%d", &r->ford);
	}
	else if(j == pos[11]) {
	    sscanf(c, "%d", &r->zp);
	}
	else if(j == pos[12]) {
	    r->ftype.assign(c);
	}
	else if(j == pos[13]) {
	    r->group.assign(c);
	}
	j++;
    }
}

static bool
findParTable(FILE *fp, const char *name)
{
    char line[501], *c, buf[100];

    while(get_non_blank(fp, line, 500) != EOF)
    {
	if((c = strstr(line, "BeginTable")) != NULL &&
		sscanf(c+10, "%s", buf) == 1 && !strcmp(buf, name))
	{
	    return true;
	}
    }
    return false;
}

static bool
getVariablePositions(FILE *fp, int num, const char **variables, int *pos)
{
    char line[501], *c, *tok, *last;
    int i, j, err;

    line[0] = '\0';
    while(!(err = stringGetLine(fp, line, 500)) &&
	    (blankLine(line) || (line[0] == '#' && line[1] == '!')));
    if(err == EOF) return false;

    for(i = 0; i < num; i++)  pos[i] = -1;

    tok = line;
    j = 0;
    while((c = strtok_r(tok, "| \t", &last)) != NULL)
    {
	tok = NULL;
	for(i = 0; i < num; i++) {
	    if(!strcasecmp(variables[i], c)) {
		pos[i] = j;
		break;
	    }
	}
	j++;
    }
    return true;
}

static int
get_non_blank(FILE *fp, char *line, int n)
{
    int err;

    while(!(err = stringGetLine(fp, line, n)) && blankLine(line));
    return err;
}

static bool
blankLine(const char *line)
{
    if(line[0] == '#' && line[1] != '!') return true;

    while(*line != '\0' && isspace((int)(*line))) line++;

    return (*line == '\0') ? true : false;
}

/** Read all beam recipe groups.
 *  @param[in] recipe_directory the recipe directory.
 *  @param[in] recipe_directory2 an optional second recipe directory.
 *  @param[out] groups the beam groups.
 *  @returns  the number of groups read. Returns -1 if an error occurred.
 *  Use GError::getMessage to obtain the error message.
 *  @throws GERROR_MALLOC_ERROR
 */
int Beam::readGroups(const string &recipe_directory,
			const string &recipe_directory2,
			vector<BeamGroup> &groups)
{
    groups.clear();

    if( !recipe_directory.empty() )
    {
	if(read_groups(recipe_directory, groups) < 0) return -1;
    }
    if( !recipe_directory2.empty() )
    {
	if(read_groups(recipe_directory2, groups) < 0) return -1;
    }
    return (int)groups.size();
}

static int
read_groups(const string &recipe_directory, vector<BeamGroup> &groups)
{
    DIR		*dirp;
    char	path[MAXPATHLEN+1];
    struct dirent *dp;

    groups.clear();
    snprintf(path, sizeof(path), "%s/beam", recipe_directory.c_str());
    if((dirp = opendir(path)) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("Cannot open: %s\n%s", path, strerror(errno));
	}
	else {
	    GError::setMessage("Cannot open: %s", path);
	}
	return -1;
    }

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	getGroups(dp->d_name, path, groups);
    }
    closedir(dirp);

    return (int)groups.size();
}

static bool
getGroups(const char *file, const char *dir, vector<BeamGroup> &groups)
{
    char	path[MAXPATHLEN+1], net[100];
    int		n;
    FILE	*fp;
    DIR		*d;

    n = (int)strlen(file);

    if(n < 10 || strcasecmp(file+n-9, "-beam.par")) return true;

    strncpy(net, file, n-9);
    net[n-9] = '\0';

    snprintf(path, sizeof(path), "%s/%s", dir, file);

    if((d = opendir(path)) != NULL) {
	closedir(d);
    }
    else
    {
	if((fp = fopen(path, "r")) == NULL)
	{
	    file_warn(path);
	}
	else if(readGroupFile(fp, dir, path, net, groups) < 0)
	{
	    GError::setMessage("Error reading groups in %s", path);
	}
	fclose(fp);
    }
    return true;
}

static int
readGroupFile(FILE *fp, const char *dir, const char *path, const char *net,
		vector<BeamGroup> &groups)
{
    int pos, ngroups, path_q;
    char line[501];
    const char *variables[] = {"group"};
    BeamGroup g;

    if( !findParTable(fp, "beam-group") ) return -1;

    if( ! getVariablePositions(fp, 1, variables, &pos) || pos == -1 )
    {
	return -1;
    }

    path_q = stringToQuark(path);

    ngroups = 0;
    while(get_non_blank(fp, line, 500) != EOF && !strstr(line, "EndTable") )
    {
	stringcpy(g.net, net, sizeof(g.net));
	g.path = path_q;
	stringcpy(g.group, stringTrim(line), sizeof(g.group));
	if(Beam::readGroup(dir, net, g.group, g.sta) < 0) {
	    logErrorMsg(LOG_ERR, GError::getMessage());
	}
	groups.push_back(g);
	ngroups++;
    }
    return ngroups;
}

/** Read the beam recipe group for the input BeamRecipe.
 *  @param[in] recipe read the group for this recipe.
 *  @param[out] beam_sta return a BeamSta structure for each station in the
 *	group.
 *  @returns  the number of stations in the group; the number of elements in
 *	(*beam_sta)[]. Returns -1 is an error occurred. Use GError::getMessage
 *	to obtain the error message.
 *  @throws GERROR_MALLOC_ERROR
 */
int Beam::getGroup(BeamRecipe &recipe, vector<BeamSta> &beam_sta)
{
    char path[MAXPATHLEN+1];
    const char *file;
    int i;

    beam_sta.clear();
    file = quarkToString(recipe.path);
    stringcpy(path, file, sizeof(path));
    for(i = (int)strlen(path)-1; i > 0 && path[i] != '/'; i--);
    while(i > 0 && path[i] == '/') i--;
    while(i > 0 && path[i] != '/') i--;
    path[i] = '\0';

    return readGroup(path, recipe.net, recipe.group, beam_sta);
}

/** Read a beam recipe group for the input group name. The filename is
 *  constructed with the following code:
\code
    snprintf(path, sizeof(path), "%s/%s-beam.par", recipe_directory, net);
\endcode
 *  @param[in] recipe_directory read the group for this recipe.
 *  @param[in] net the network name
 *  @param[in] group the group name
 *  @param[out] beam_sta returns a BeamSta structure for each station in the
 *	group.
 *  @returns  the number of stations in the group; the number of elements in
 *	(*beam_sta)[]. Returns -1 is an error occurred. Use GError::getMessage
 *	to obtain the error message.
 *  @throws GERROR_MALLOC_ERROR
 */
int Beam::readGroup(const string &recipe_directory, const string &net,
		const string &group, vector<BeamSta> &beam_sta)
{
    char path[MAXPATHLEN+1];
    char line[501], *c, *tok, *last;
    const char *variables[] = { "sta", "chan", "wgt"};
    int j, pos[3];
    BeamSta b;
    FILE *fp;

    beam_sta.clear();
    snprintf(path, sizeof(path), "%s/%s-beam.par", recipe_directory.c_str(),
		net.c_str());

    if((fp = fopen(path, "r")) == NULL)
    {
	file_warn(path);
	if(fp != NULL) fclose(fp);
	return -1;
    }

    if( !findParTable(fp, group.c_str()) ) {
	fclose(fp);
	GError::setMessage("Cannot find group %s in %s", group.c_str(), path);
	return -1;
    }

    if( ! getVariablePositions(fp, 3, variables, pos) || pos[0] == -1
		|| pos[1] == -1 || pos[2] == -1)
    {
	GError::setMessage("Error reading %s\nExpecting columns sta chan wgt", 
			path);
	fclose(fp);
	return -1;
    }

    while(get_non_blank(fp, line, 500) != EOF && !strstr(line, "EndTable") )
    {
	stringcpy(b.sta, "", sizeof(b.sta));
	stringcpy(b.chan, "", sizeof(b.chan));
	b.wgt = 0;

	tok = line;
	j = 0;
	while((c = strtok_r(tok, "| \t", &last)) != NULL)
	{
	    tok = NULL;

	    if(j == pos[0]) {
		stringcpy(b.sta, c, sizeof(b.sta));
	    }
	    else if(j == pos[1]) {
		stringcpy(b.chan, c, sizeof(b.sta));
	    }
	    else if(j == pos[2]) {
		sscanf(c, "%lf", &b.wgt);
	    }
	    j++;
	}
	beam_sta.push_back(b);
    }
    fclose(fp);
	
    return (int)beam_sta.size();
}

/** Add a beam recipe group. The path to the group file is constructed with:
\code
    snprintf(path, sizeof(path), "%s/%s-beam.par", recipe_directory, net);
\endcode
 *  The file is created if it does not exist. If the group file already exists,
 *  the file is edited to contain the input group.
 *  @param[in] net the network name.
 *  @param[in] group the group name.
 *  @param[in] sta vector of BeamSta structures for each station in the group.
 *  @param[in] recipe_directory the recipe directory.
 *  @returns true for success. Returns false if an error occurred.
 */
bool Beam::addGroup(const string &net, string &group, vector<BeamSta> &sta,
		const string &recipe_directory)
{
    char path[MAXPATHLEN+1];
    struct stat buf;

    snprintf(path, sizeof(path), "%s/beam/%s-beam.par",
		recipe_directory.c_str(), net.c_str());

    if(!stat(path, &buf)) /* file exists */
    {
	return add_group(path, group, sta);
    }
    else {
	return new_group_file(net, path, group, sta);
    }
}

static bool
add_group(const char *path, string &group, vector<BeamSta> &sta)
{
    char tmp_path[MAXPATHLEN+1];
    char line[501];
    const char *variables[] = { "group" };
    int i, insert_pos, pos;
    FILE *fp, *fptmp;

    if((fp = fopen(path, "r")) == NULL)
    {
	file_warn(path);
	return false;
    }
    stringcpy(tmp_path, path, sizeof(tmp_path));
    strcat(tmp_path, ".gtl");

    if((fptmp = fopen(tmp_path, "w")) == NULL)
    {
	file_warn(tmp_path);
	fclose(fp);
	return false;
    }
    if( !findParTable(fp, "beam-group") ) {
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	GError::setMessage("Cannot find beam-group in %s", path);
	return false;
    }
    if( ! getVariablePositions(fp, 1, variables, &pos) || pos == -1 )
    {
	GError::setMessage("Error reading %s\nExpecting column group", path);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }

    while(get_non_blank(fp, line, 500) != EOF && !strstr(line, "EndTable") );

    if(strstr(line, "EndTable") == NULL) {
	GError::setMessage("Error reading %s\nExpecting #!EndTable", path);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }
    insert_pos = ftell(fp) - strlen(line)-1;
    fseek(fp, 0, 0);
    for(i = 0; i < insert_pos; i++) {
	fputc(fgetc(fp), fptmp);
    }
    fprintf(fptmp, " %s\n", group.c_str());

    while((i = fgetc(fp)) != EOF) fputc(i, fptmp);

    fprintf(fptmp, "\n#!BeginTable %s\n|sta       |chan        |wgt|\n",
		group.c_str());
    for(i = 0; i < (int)sta.size(); i++) {
	fprintf(fptmp, " %-10.10s %-12.12s %.2f\n", sta[i].sta, sta[i].chan,
		sta[i].wgt);
    }
    fprintf(fptmp, "#!EndTable\n");
    fclose(fptmp);
    fclose(fp);

    if(unlink(path)) {
	GError::setMessage("Cannot change %s\n", path);
	unlink(tmp_path);
	return false;
    }
    if(rename(tmp_path, path)) {
	GError::setMessage("Error editing %s\n", path);
	return false;
    }
    return true;
}

/** Delete a group. Remove the input group from its group file.
 *  @param[in] group a BeamGroup structure.
 *  @returns true for success. Returns false if an error occurred.
 */
bool Beam::deleteGroup(BeamGroup *group)
{
    char tmp_path[MAXPATHLEN+1];
    char line[501], tmp[501], *c;
    const char *variables[] = { "group" }, *file;
    int i, insert_pos, skip_pos, pos;
    FILE *fp, *fptmp;

    file = quarkToString(group->path);
    if((fp = fopen(file, "r")) == NULL)
    {
	file_warn(file);
	return false;
    }
    snprintf(tmp_path, MAXPATHLEN+1, "%s.gtl", file);

    if((fptmp = fopen(tmp_path, "w")) == NULL)
    {
	file_warn(tmp_path);
	fclose(fp);
	return false;
    }
    if( !findParTable(fp, "beam-group") ) {
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	GError::setMessage("Cannot find beam-group in %s", group->path);
	return false;
    }
    if( ! getVariablePositions(fp, 1, variables, &pos) || pos == -1 )
    {
	GError::setMessage("Error reading %s\nExpecting column group",
				group->path);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }

    while(get_non_blank(fp, line, 500) != EOF && !strstr(line, "EndTable") )
    {
	stringcpy(tmp, line, sizeof(tmp));
	if(!strcmp(stringTrim(tmp), group->group))
	{
	    skip_pos = ftell(fp);
	    insert_pos = ftell(fp) - strlen(line)-1;
	    fseek(fp, 0, 0);
	    for(i = 0; i < insert_pos; i++) {
		fputc(fgetc(fp), fptmp);
	    }
	    fseek(fp, skip_pos, 0);
	    break;
	}
    }

    if(strstr(line, "EndTable") != NULL) {
	GError::setMessage("Error reading %s\nCannot find group %s",
			group->path, group->group);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }
    pos = ftell(fp);

    while(get_non_blank(fp, line, 500) != EOF)
    {
	if((c = strstr(line, "BeginTable")) != NULL &&
		sscanf(c+10, "%s", tmp) == 1 && !strcmp(tmp, group->group))
	{
	    insert_pos = ftell(fp) - strlen(line)-1;
	    fseek(fp, pos, 0);
	    for(i = pos; i < insert_pos; i++) {
		fputc(fgetc(fp), fptmp);
	    }
	    while(get_non_blank(fp, line, 500) != EOF &&
			!strstr(line, "EndTable") );
	    while((i = fgetc(fp)) != EOF) fputc(i, fptmp);
	}
    }
    fclose(fptmp);
    fclose(fp);

    if(unlink(file)) {
	GError::setMessage("Cannot change %s\n", file);
	unlink(tmp_path);
	return false;
    }
    if(rename(tmp_path, file)) {
	GError::setMessage("Error editing %s\n", file);
	return false;
    }
    return true;
}

static int
new_group_file(const string &net, const char *path, string &group,
		vector<BeamSta> &sta)
{
    int i;
    FILE *fp;

    if((fp = fopen(path, "w")) == NULL)
    {
	file_warn(path);
	return -1;
    }
    fprintf(fp, "beam-sta=%s\n\n", net.c_str());
    fprintf(fp, "#!BeginTable beam-group\n|group     |\n%s\n#!EndTable\n\n",
		group.c_str());
    fprintf(fp, "#!BeginTable %s\n|sta       |chan        |wgt|\n",
		group.c_str());

    for(i = 0; i < (int)sta.size(); i++) {
	fprintf(fp, " %-10.10s %-12.12s %.2f\n", sta[i].sta, sta[i].chan,
		sta[i].wgt);
    }
    fprintf(fp, "#!EndTable\n\n");
    fclose(fp);
    return 1;
}

/** Read all origin beam recipes.
 *  @param[in] recipe_directory the recipe directory.
 *  @param[in] recipe_directory2 an optional second recipe directory.
 *  @param[out] recipes an array of origin BeamRecipe structures.
 *  @returns the number of recipes. Returns -1 is an error occurred.
 */
int Beam::readOriginRecipes(const string &recipe_directory,
		const string &recipe_directory2, const string &selected,
		vector<BeamRecipe> &recipes)
{
    int num;

    if((num = readRecipes(recipe_directory, recipe_directory2, recipes,0)) <= 0)
    {
	return num;
    }
    selectRecipes(recipes, selected);
    return num;
}

/** Read all detection beam recipes.
 *  @param[in] recipe_directory the recipe directory.
 *  @param[in] recipe_directory2 an optional second recipe directory.
 *  @param[out] recipes an array of detection BeamRecipe structures.
 *  @returns the number of recipes. Returns -1 is an error occurred.
 */
int Beam::readDetectionRecipes(const string &recipe_directory,
			const string &recipe_directory2, const string &selected,
			vector<BeamRecipe> &recipes)
{
    int num;

    if((num = readRecipes(recipe_directory, recipe_directory2, recipes,1)) <= 0)
    {
	return num;
    }
    selectRecipes(recipes, selected);
    return num;
}

static void
selectRecipes(vector<BeamRecipe> &r, const string &recipes)
{
    int i;
    char *selected, *tok, *last, name[100];
    char *prop = strdup(recipes.c_str());
    tok = prop;
    while((selected = strtok_r(tok, ",", &last)) != NULL)
    {
	tok = NULL;
	for(i = 0; i < (int)r.size(); i++)
	{
	    snprintf(name, sizeof(name), "%s/%s",
			r[i].net.c_str(), r[i].name.c_str());
	    if(!strcmp(selected, name)) {
		r[i].selected = true;
	    }
	}
    }
    free(prop);
}

static int
readRecipes(const string &recipe_directory, const string &recipe_directory2,
		vector<BeamRecipe> &recipes, int type)
{
    recipes.clear();

    if( !recipe_directory.empty() )
    {
	read_recipes(type, recipe_directory, recipes);
    }
    if( !recipe_directory2.empty() )
    {
	read_recipes(type, recipe_directory2, recipes);
    }
    return (int)recipes.size();
}

static int
read_recipes(int type, const string &recipe_directory,
		vector<BeamRecipe> &recipes)
{
    char path[MAXPATHLEN+1];
    DIR	 *dirp;
    struct dirent *dp;

    if(type == 0) {
	snprintf(path, sizeof(path), "%s/beam/originbeam",
		recipe_directory.c_str());
    }
    else {
	snprintf(path, sizeof(path), "%s/beam/detection",
		recipe_directory.c_str());
    }
    recipes.clear();

    if((dirp = opendir(path)) == NULL)
    {
	if(errno > 0) {
	    GError::setMessage("Cannot open: %s\n%s", path, strerror(errno));
	}
	else {
	    GError::setMessage("Cannot open: %s", path);
	}
	return -1;
    }

    for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
    {
	getRecipes(recipe_directory, dp->d_name, path, recipes);
    }
    closedir(dirp);

    return (int)recipes.size();
}

static int
getRecipes(const string &recipe_directory, const char *file, const char *dir,
		vector<BeamRecipe> &r)
{
    char	path[MAXPATHLEN+1], net[100];
    int		n;
    FILE	*fp;
    DIR		*d;

    n = (int)strlen(file);

    if(n < 10 || strcasecmp(file+n-9, "-beam.par")) return 0;

    strncpy(net, file, n-9);
    net[n-9] = '\0';

    snprintf(path, sizeof(path), "%s/%s", dir, file);

    if((d = opendir(path)) != NULL) {
	closedir(d);
	return 0;
    }

    if((fp = fopen(path, "r")) == NULL) {
	file_warn(path);
	return 0;
    }

    n = Beam::readFile(recipe_directory, fp, net, path, sizeof(path), r);
    if(n < 0) {
	GError::setMessage("Error reading recipes in %s", path);
	n = 0;
    }
    fclose(fp);

    return n;
}

/** Change or delete a beam recipe.
 *  @param[in] recipe the altered recipe.
 *  @param[in] delete_recipe if true, delete the recipe. If false, save the
 *   current recipe parameters.
 *  @returns true for success. Returns false if an error occurred.
 */
bool Beam::changeRecipe(BeamRecipe *recipe, bool delete_recipe)
{
    int i, len, insert_pos, skip_pos, pos[14];
    char line[501], tmp_path[MAXPATHLEN+1];
    const char *file;
    const char *variables[] = {
	"name", "type", "rot", "std", "snr", "azi", "slow", "phase",
	"flo", "fhi", "ford", "zp", "ftype", "group"
    };
    BeamRecipe r;
    FILE *fp, *fptmp;

    file = quarkToString(recipe->path);
    if((fp = fopen(file, "r")) == NULL)
    {
	file_warn(file);
	return false;
    }
    snprintf(tmp_path, MAXPATHLEN+1, "%s.gtl", file);

    if((fptmp = fopen(tmp_path, "w")) == NULL)
    {
	file_warn(tmp_path);
	fclose(fp);
	return false;
    }

    if( ! findParTable(fp, "beam-recipe") ) {
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }

    if( ! getVariablePositions(fp, 14, variables, pos) || pos[0] == -1)
    {
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }

    while(get_non_blank(fp, line, 500) != EOF && !strstr(line, "EndTable") )
    {
	len = strlen(line);
	parseLine(line, pos, &r);

	if(!r.name.compare(recipe->name)) {
	    skip_pos = ftell(fp);
	    insert_pos = skip_pos - len - 1;
	    fseek(fp, 0, 0);
	    for(i = 0; i < insert_pos; i++) {
		fputc(fgetc(fp), fptmp);
	    }
	    if( ! delete_recipe ) {
		write_recipe(fptmp, pos, recipe);
	    }

	    fseek(fp, skip_pos, 0);
	    while((i = fgetc(fp)) != EOF) fputc(i, fptmp);

	    fclose(fptmp);
	    fclose(fp);

	    if(unlink(file)) {
		GError::setMessage("Cannot change %s\n", recipe->path);
		unlink(tmp_path);
		return false;
	    }
	    if(rename(tmp_path, file)) {
		GError::setMessage("Error editing %s\n", file);
		return -1;
	    }
	    return true;
	}
    }
    GError::setMessage("Error editing %s\n", file);
    return false;
}

static void
write_recipe(FILE *fp, int *pos, BeamRecipe *recipe)
{
    int i, j;

    for(i = 0; i < 14; i++) {
	for(j = 0; j < 14; j++) if(pos[j] == i)
	{
	    switch(j) {
		case 0: fprintf(fp, " %-9.9s",recipe->name.c_str()); break;
		case 1: fprintf(fp, " %-4.4s",recipe->beam_type.c_str()); break;
		case 2: fprintf(fp, " %-4.4s", recipe->rot.c_str()); break;
		case 3: fprintf(fp, " %-3d", recipe->std); break;
		case 4: fprintf(fp, " %5.2f", recipe->snr); break;
		case 5: fprintf(fp, " %-5.1f", recipe->azi); break;
		case 6: fprintf(fp, " %-6.3f", recipe->slow); break;
		case 7: fprintf(fp, " %-9.9s", recipe->phase.c_str()); break;
		case 8: fprintf(fp, " %-5.2f", recipe->flo); break;
		case 9: fprintf(fp, " %-5.2f", recipe->fhi); break;
		case 10: fprintf(fp, " %-4d", recipe->ford); break;
		case 11: fprintf(fp, " %-2d", recipe->zp); break;
		case 12: fprintf(fp, " %-5.5s", recipe->ftype.c_str());break;
		case 13: fprintf(fp, " %s", recipe->group.c_str()); break;
	    }
	}
    }
    fprintf(fp, "\n");
}

/** Change or delete a beam recipe.
 *  @param[in] recipe_directory the recipe directory
 *  @param[in] recipe the recipe to add.
 *  @param[in] origin_beam true for an origin beam recipe, false for a
 *	detection beam recipe.
 *  @returns true for success. Returns false if an error occurred.
 */
bool Beam::addRecipe(const string &recipe_directory, BeamRecipe *recipe,
			bool origin_beam)
{
    char path[MAXPATHLEN+1];
    struct stat buf;

    if(origin_beam) {
	snprintf(path, sizeof(path), "%s/beam/originbeam/%s-beam.par",
			recipe_directory.c_str(), recipe->net.c_str());
    }
    else {
	snprintf(path, sizeof(path), "%s/beam/detection/%s-beam.par",
			recipe_directory.c_str(), recipe->net.c_str());
    }

    if(!stat(path, &buf)) /* file exists */
    {
	return add_recipe(path, recipe);
    }
    else if(!stat(recipe_directory.c_str(), &buf)) {
	if(S_ISDIR(buf.st_mode)) {
	    return new_recipe_file(recipe_directory, recipe, origin_beam);
	}
	else {
	    GError::setMessage("%s is not a directory.",
			recipe_directory.c_str());
	    return false;
	}
    }
    else {
	GError::setMessage("%s not found.", recipe_directory.c_str());
	return false;
    }
}

static bool
add_recipe(const char *path, BeamRecipe *recipe)
{
    int i, j, k, insert_pos, pos[14];
    char line[501], tmp_path[MAXPATHLEN+1];
    const char *variables[] = {
	"name", "type", "rot", "std", "snr", "azi", "slow", "phase",
	"flo", "fhi", "ford", "zp", "ftype", "group"
    };
    FILE *fp, *fptmp;

    if((fp = fopen(path, "r")) == NULL)
    {
	file_warn(path);
	return false;
    }
    snprintf(tmp_path, MAXPATHLEN+1, "%s.gtl", path);

    if((fptmp = fopen(tmp_path, "w")) == NULL)
    {
	file_warn(tmp_path);
	fclose(fp);
	return false;
    }

    if( ! findParTable(fp, "beam-recipe") ) {
	GError::setMessage("Cannot find beam-recipe in %s", path);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }

    if( ! getVariablePositions(fp, 14, variables, pos) || pos[0] == -1)
    {
	GError::setMessage("Error reading %s.", path);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }

    while(get_non_blank(fp, line, 500) != EOF && !strstr(line, "EndTable") );

    if(strstr(line, "EndTable") == NULL) {
	GError::setMessage("Error reading %s\nExpecting #!EndTable", path);
	fclose(fp);
	fclose(fptmp);
	unlink(tmp_path);
	return false;
    }
    insert_pos = ftell(fp) - strlen(line)-1;
    fseek(fp, 0, 0);
    for(i = 0; i < insert_pos; i++) {
	fputc(fgetc(fp), fptmp);
    }
    for(j = 0; j < 14; j++)
    {
	for(k = 0; k < 14; k++) if(pos[k] == j)
	{
	    switch(k) {
	    case 0: fprintf(fptmp, " %-9.9s", recipe->name.c_str()); break;
	    case 1: fprintf(fptmp, " %-4.4s", recipe->beam_type.c_str()); break;
	    case 2: fprintf(fptmp, " %-4.4s", recipe->rot.c_str()); break;
	    case 3: fprintf(fptmp, " %-3d", recipe->std); break;
	    case 4: fprintf(fptmp, " %5.2f", recipe->snr); break;
	    case 5: fprintf(fptmp, " %-5.1f", recipe->azi); break;
	    case 6: fprintf(fptmp, " %-6.3f", recipe->slow); break;
	    case 7: fprintf(fptmp, " %-9.9s", recipe->phase.c_str()); break;
	    case 8: fprintf(fptmp, " %-5.2f", recipe->flo); break;
	    case 9: fprintf(fptmp, " %-5.2f", recipe->fhi); break;
	    case 10: fprintf(fptmp, " %-4d", recipe->ford); break;
	    case 11: fprintf(fptmp, " %-2d", recipe->zp); break;
	    case 12: fprintf(fptmp, " %-5.5s", recipe->ftype.c_str());break;
	    case 13: fprintf(fptmp, " %s", recipe->group.c_str()); break;
	    }
	}
    }
    fprintf(fptmp, "\n");

    while((i = fgetc(fp)) != EOF) fputc(i, fptmp);

    fclose(fptmp);
    fclose(fp);

    if(unlink(path)) {
	GError::setMessage("Cannot change %s\n", path);
	unlink(tmp_path);
	return false;
    }
    if(rename(tmp_path, path)) {
	GError::setMessage("Error editing %s\n", path);
	return false;
    }
    return true;
}

static int
new_recipe_file(const string &recipe_directory, BeamRecipe *recipe,
		bool origin_beam)
{
    FILE *fp;
    char path[MAXPATHLEN+1];
    struct stat buf;

    snprintf(path, sizeof(path), "%s/beam", recipe_directory.c_str());
    if(stat(path, &buf) && mkdir(path, S_IRWXU)) {
	GError::setMessage("Cannot create directory %s", path);
	return -1;
    }
    if(origin_beam) {
	snprintf(path, sizeof(path), "%s/beam/originbeam",
		recipe_directory.c_str());
	if(stat(path, &buf) && mkdir(path, S_IRWXU)) {
	    GError::setMessage("Cannot create directory %s", path);
	    return -1;
	}
	snprintf(path, sizeof(path), "%s/beam/originbeam/%s-beam.par",
			recipe_directory.c_str(), recipe->net.c_str());
    }
    else {
	snprintf(path, sizeof(path), "%s/beam/detection",
		recipe_directory.c_str());
	if(stat(path, &buf) && mkdir(path, S_IRWXU)) {
	    GError::setMessage("Cannot create directory %s", path);
	    return -1;
	}
	snprintf(path, sizeof(path), "%s/beam/detection/%s-beam.par",
			recipe_directory.c_str(), recipe->net.c_str());
    }

    if((fp = fopen(path, "w")) == NULL)
    {
	file_warn(path);
	return -1;
    }
    fprintf(fp, "\npar=$(PARDIR)/beam/$(sta)-beam.par\n\n");

    fprintf(fp, "#!BeginTable beam-recipe\n");
    fprintf(fp, "|name     |type|rot |std|snr  |azi  |slow  |phase    |flo  |fhi  |ford|zp|ftype|group   |\n");

    fprintf(fp, " %-9.9s", recipe->name.c_str());
    fprintf(fp, " %-4.4s", recipe->beam_type.c_str());
    fprintf(fp, " %-4.4s", recipe->rot.c_str());
    fprintf(fp, " %-3d", recipe->std);
    fprintf(fp, " %5.2f", recipe->snr);
    fprintf(fp, " %-5.1f", recipe->azi);
    fprintf(fp, " %-6.3f", recipe->slow);
    fprintf(fp, " %-9.9s", recipe->phase.c_str());
    fprintf(fp, " %-5.2f", recipe->flo);
    fprintf(fp, " %-5.2f", recipe->fhi);
    fprintf(fp, " %-4d", recipe->ford);
    fprintf(fp, " %-2d", recipe->zp);
    fprintf(fp, " %-5.5s", recipe->ftype.c_str());
    fprintf(fp, " %s\n", recipe->group.c_str());

    fprintf(fp, "#!EndTable\n\n");
    fclose(fp);
    return 1;
}

bool Beam::beamRecipe(const string &net, const string &chan, BeamRecipe &r)
{
    if((int)origin_recipes.size() == 0) {
	readOriginRecipes(recipe_dir, recipe_dir2, origins_selected,
			origin_recipes);
    }
    int i;
    for(i = 0; i < (int)origin_recipes.size()
	&& (!DataSource::compareChan(chan, origin_recipes[i].name) ||
	strcasecmp(net.c_str(), origin_recipes[i].net.c_str())); i++);

    if(i < (int)origin_recipes.size()) {
	r = origin_recipes[i];
	return true;
    }
	
    if((int)detection_recipes.size() == 0) {
	readDetectionRecipes(recipe_dir, recipe_dir2, detections_selected,
			detection_recipes);
    }
    for(i = 0; i < (int)detection_recipes.size()
	&& (!DataSource::compareChan(chan, detection_recipes[i].name) ||
	strcasecmp(net.c_str(), detection_recipes[i].net.c_str())); i++);

    if(i < (int)detection_recipes.size()) {
	r = detection_recipes[i];
	return true;
    }

    return false;
}

vector<BeamRecipe> * Beam::getOriginRecipes(bool force_read)
{
    if(force_read) {
	origin_recipes.clear();
    }
    if((int)origin_recipes.size() == 0) {
	readOriginRecipes(recipe_dir, recipe_dir2, origins_selected,
			origin_recipes);
    }
    return &origin_recipes;
}

vector<BeamRecipe> * Beam::getDetectionRecipes(bool force_read)
{
    if(force_read) {
	detection_recipes.clear();
    }
    if((int)detection_recipes.size() == 0) {
	readDetectionRecipes(recipe_dir, recipe_dir2, detections_selected,
			detection_recipes);
    }
    return &detection_recipes;
}

bool Beam::getSelectedOriginRecipe(const string &net, BeamRecipe &b)
{
    vector<BeamRecipe> *r = getOriginRecipes();
    for(int i = 0; i < (int)r->size(); i++) {
	if(r->at(i).selected && !strcasecmp(net.c_str(),r->at(i).net.c_str())) {
	    b = r->at(i);
	    return true;
	}
    }
    return false;
}
