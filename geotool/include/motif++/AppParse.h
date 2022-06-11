#ifndef _APP_PARSE_H
#define _APP_PARSE_H

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "motif++/Component.h"
#include "widget/TableListener.h"

typedef void (*CreateClass)(const string &name, Component *parent,
		const string &params);
typedef struct
{
    char *class_name;
    CreateClass create;
} CreateCompStruct;

class Alias : public Gobject
{
    public:
	string alias;
	string name;
	Alias(string a, string n) : alias(a), name(n) { }
	~Alias(void) { }
};

enum ConditionalState
{
    CONDITIONAL_NONE,
    CONDITIONAL_IF_TRUE,
    CONDITIONAL_IF_FALSE,
    CONDITIONAL_ELSEIF_TRUE,
    CONDITIONAL_ELSEIF_FALSE,
    CONDITIONAL_ELSE_TRUE,
    CONDITIONAL_ELSE_FALSE,
    CONDITIONAL_SKIP
};
enum ForeachState
{
    FOREACH_NONE,
    FOREACH_READ,
    FOREACH_SKIP,
    FOREACH_BREAK
};
enum TableCBType
{
    TABLE_ADDED,
    TABLE_DELETED,
    TABLE_MODIFIED
};

struct line_struct {char *line; int line_number;};

class Foreach {
    public:
    char *type;
    char *prefix;
    char *name;
    char *array_save;
    char *array_prop;
    char *array_last;
    enum ForeachState mode;
    vector<struct line_struct> lines;
    vector<Alias *> aliases;
    int start;
    int end;
    int i;

    Foreach() : type(NULL), prefix(NULL), name(NULL), array_save(NULL),
	array_prop(NULL), array_last(NULL), mode(FOREACH_NONE),
	lines(), aliases(), start(0), end(0), i(0)
    {
    }
    Foreach(const Foreach &f) : type(NULL), prefix(NULL), name(NULL),
	array_save(NULL), array_prop(NULL), array_last(NULL),
	mode(f.mode), lines(f.lines), aliases(f.aliases),
	start(f.start), end(f.end), i(f.i)
    {
	if(f.type) type = strdup(f.type);
	if(f.prefix) prefix = strdup(f.prefix);
	if(f.name) name = strdup(f.name);
	if(f.array_save) array_save = strdup(f.array_save);
	if(f.array_prop) array_prop = strdup(f.array_prop);
	if(f.array_last) array_last = strdup(f.array_last);
    }

    Foreach & operator=(const Foreach &f) {
	type = f.type ? strdup(f.type) : NULL;
	prefix = f.prefix ? strdup(f.prefix) : NULL;
	name = f.name ? strdup(f.name) : NULL;
	array_save = f.array_save ? strdup(f.array_save) : NULL;
	array_prop = f.array_prop ? strdup(f.array_prop) : NULL;
	array_last = f.array_last ? strdup(f.array_last) : NULL;
	mode = f.mode;
	lines = f.lines;
	aliases = f.aliases;
	start = f.start;
	end = f.end;
	i = f.i;
	return *this;
    }

    ~Foreach() {
	clear();
    }
    void clear() {
	mode = FOREACH_NONE;
	Free(type);
	Free(prefix);
	Free(name);
	Free(array_save);
	Free(array_prop);
	array_last = NULL;
	for(int j = 0; j < (int)lines.size(); j++) Free(lines[j].line);
	lines.clear();
	for(int j = 0; j < (int)aliases.size(); j++) {
	    delete aliases[j];
	}
	aliases.clear();
    }
};

enum ItemType
{
    INT_ITEM,
    DOUBLE_ITEM,
    STRING_ITEM
};

class FileState
{
    public:
    vector<Alias *> aliases;
    vector<Foreach *> foreach;
    int foreach_blocks;
    int if_blocks;
    ghashtable<string *> variables;
    vector<enum ConditionalState> cs;

    FileState() : aliases(), foreach(), foreach_blocks(0), if_blocks(0),
	variables(), cs()
    {
    }
    FileState(const FileState &f) : aliases(f.aliases), foreach(f.foreach),
	foreach_blocks(f.foreach_blocks), if_blocks(f.if_blocks),
	variables(), cs(f.cs)
    { }
    FileState & operator=(const FileState &f) {
	aliases = f.aliases;
	foreach = f.foreach;
	foreach_blocks = f.foreach_blocks;
	if_blocks = f.if_blocks;
	cs = f.cs;
	variables = f.variables;
	return *this;
    }

    ~FileState() {
	for(int i = 0; i < (int)aliases.size(); i++) {
	    delete aliases[i];
	}
	cs.clear();
    }
};

 /* @ingroup libmotif
 */
class AppParse : public Component
{
    friend class Foreach;

    public:

	AppParse(const string &name, Component *parent, int *argc,
			const char **argv, const string &installation_dir);
	~AppParse(void);

	/** Get installation directory */
	const char *installationDir(void) { return install_dir.c_str(); }

	static void * readInput(void *client_data);

	bool parseLine(const char *line) {
	    string msg;
	    return parseLine(line, msg);
	}
	bool parseLine(const char *line, string &msg);
	bool parseCommand(char *line, int line_size, string &msg);
	void parseCmdCallback(void);
	bool parseCallback(const char *script, vector<ParseFileInfo> &pf,
			string &msg);
	ParseCmd create(Component *comp, const string &cmd, string &msg);
	ParseCmd parseXArgs(Component *comp, const string &name,
			const string &cmd, string &msg);

        /** If true, the command parser is running without graphics. */
        static bool parseOnly(void) { return getAppParse()->parse_only; }
	char *getFullPath(const string &file);
        static AppParse *getAppParse(void);

	static char * fullPath(const string &file) {
	    return getAppParse()->getFullPath(file); }
	static void getFullPath(const string &file, string &path) {
		char *c = fullPath(file);
		if(c) { path.assign(c); free(c); }
	}

	static void addCreateMethod(const string &class_name,
		CreateClass method);

	static void putParseProperty(const string &name, const string &value) {
	    getAppParse()->putGlobalVariable(name, value);
	}

#ifdef __STDC__
	void printParseError(const char *format, ...);
#else
        void printParseError(va_alist);
#endif
        const char *resourceName(void) { return resource_name.c_str(); }

	void putVariable(const string &name, const string &value);
	void removeVariable(const string &name);
	ParseVar getVariable(const string &name, char **var);

	void putGlobalVariable(const string &name, const string &value) {
	    global_variables.put(name, value);
	}
	void removeGlobalVariable(const string &name) {
	    global_variables.remove(name);
	}
	char *getGlobalVariable(const string &name) {
	    string *s;
	    if(global_variables.get(name, &s)) {
		return strdup(s->c_str());
	    }
	    return NULL;
	}
	bool reservedName(const string &name);
	void addReservedName(const string &name);
	bool addMessageHandler(const string &c);
	void parseIPCMessage(const char *msg_id, const char *msg,
		const char *msg_class);
	bool sendMessage(Component *top, Foreach *fr, char *line);
	void setTableCallback(const string &cmd, enum TableCBType type);
	void tableModified(TableListenerCallback *tc, const char *command);
	void addTableCB(CssTableClass *table) {
	    TableListenerCallback tc;
	    tc.num_members = 0;
	    tc.table = table;
	    tableModified(&tc, "add");
	}
	CssTableClass *getModifiedTable(void) { return modified_table; }
	bool readParseLine(char *line, int line_size);
	void parseWait(long msecs);
	void parseContinue(void);

	virtual void stop(int code);

    protected:
        int nargcp;     //!< the number of command-line arguments
        const char **arg_v; //!< the command-line arguments
        XtAppContext app_context; //!< the Application Context.
        string application_class; //!< the X-resource application class name
	string install_dir; //!< the program installation directory.

	bool parse_only; //!< if true, commands are parsed without graphics.
	/** The parse string from the command line. */
	vector<char *> command_line_parse_string;

	vector<ParseFileInfo> parse_file;

	string print_file;
	string write_file;
	FILE *print_fp;
	FILE *write_fp;
	FILE *get_line_fp;
	bool standard_input;
	pthread_t thread;
	sem_t sem_stdin;
	sem_t sem_work;
	bool input_ready;
	bool stop_flag; //!< stop-flag.
	char parse_line[2000];
	ScriptCallback table_modify;
	ScriptCallback table_delete;
	ScriptCallback table_add;
	CssTableClass *modified_table;

	vector<CreateCompStruct> create_methods;

	virtual void cleanUp(void) {}

	bool getLine(FILE *fp, char *line, int line_size);
	void listAliases(void);
	bool unalias(char *line);
	bool setAlias(const char *line);
	void applyAliases(char *line, int line_size);
	bool parseFile(char *file, string &msg);
	int conditionalCmd(Component *top, char *line, string &msg);
	bool foreachCmd(Component *top, char *line, string &msg);
	bool foreachEnd(Component *top, char *line, string &msg);
	bool sprintCmd(Component *top, char *line, char sep, string &msg);
	bool printCmd(Component *top, char *line, string &msg, char sep,
			char **to_string=NULL, bool isprintf=false);
	bool formattedPrint(Component *top, char *o, int n, char *fmt, char *c,
			char *sep);
	bool printOpenFile(const string &c);
	bool writeOpenFile(const string &c);
	bool checkLine(char *line, string &msg);
	void getScriptFiles(void);
	void initCreateMethods(void);

	// cannot actually copy the AppParse instance. This is just to avoid
	// compiler warnings from the -Weffc++ option on this file
	AppParse(const AppParse &);
	AppParse & operator=(const AppParse &a) { return *this; }

    private:
	string resource_name; //!< the program name for X-resources

	ghashtable<string *> global_variables;
	vector<int> reserved_names;

	vector<FileState *> file_states;
	FileState *start_fs;

	typedef struct
	{
	    char *c;
	    char *func;
	    char oper;
	    bool minus;
	    bool not_sign;
	    bool flag;
	    ItemType type;
	    long l;
	    double d;
	} Item;

	bool processForeach(Component *top, Foreach *fr, string &msg);
	bool doSubstitutions(Component *top, char *line, char *sline,
				int sline_size, bool quote=false);
	bool assignString(Component *top, char *line);
	bool assignVariable(Component *top, char *line, ParseVar *ret);
	bool isCommand(char *line);
	ParseVar parseWithPrefix(Component *top, char *name, string &msg);
	ParseVar parseExpression(Component *top, char *line, bool logical,
				string &msg);
	ParseVar parseParen(Component *top, Item *items, int *nitems,
				int i1, int i2, string &msg);
	ParseVar startArray(char *name);
	ParseVar nextArray(string &msg);
	bool isEnd(char *line);
	int parseTerms(char *line, Item *items, int max_items);
	ParseVar checkSyntax(int *nitems, Item *items, bool logical,char *line);
	ParseVar findMinusSigns(int *nitems, Item *items, char *line);
	ParseVar findNotSigns(int *nitems, Item *items, char *line);
	ParseVar evaluateTerms(Component *top, int nitems, Item *items,
				string &msg);
	bool maxabs(Item *items, int i1, int nargs);
	bool writeString(Item *items, int i1, int nargs);
	bool writeFloat(Item *items, int i1, int nargs);
	bool writeDouble(Item *items, int i1, int nargs);
	bool writeInt(Item *items, int i1, int nargs);
	bool writeLong(Item *items, int i1, int nargs);
	bool writeArray(Item *items, int i1, int nargs);
	bool callFunc(Component *top, Item *items, int i1, int *nitems);
	bool exportVariable(char *name);
};

#endif
