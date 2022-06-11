#ifndef _COMMAND_PARSER_H
#define _COMMAND_PARSER_H
#include <stdio.h>

/** parseCmd return code.
 *  @ingroup libgobject
 */
enum ParseCmd {
    COMMAND_PARSED,	//!< the command was parsed.
/** the command name was recognized, but one or more arguments were invalid. */
    ARGUMENT_ERROR,
    COMMAND_NOT_FOUND	//!< the command name was not found.
};

/** parseVariable return code.
 *  @ingroup libgobject
 */
enum ParseVar {
    VARIABLE_NOT_FOUND,	//!< the variable name was not found.
    STRING_RETURNED,	//!< the value is the requested string
    VARIABLE_TRUE,	//!< the variable was a method that returned true
    VARIABLE_FALSE,	//!< the variable was a method that returned false
    FOREACH_MORE,	//!< there are more objects in the array
    FOREACH_NO_MORE,	//!< there are no more objects in the array
    VARIABLE_ERROR	//!< the variable name was incorrectly specified
};

/** Virtual base class for command parsers
 *  @ingroup libmotif
 */
class CommandParser
{
    public:
	virtual ~CommandParser() {}

	/** Parse string commands.
	  * @param[in] cmd a single line of input.
	  * @param[in,out] msg a string to hold a message.
	  * @param[in] msg_len the length of msg.
	  * @returns COMMAND_PARSED if the cmd was parsed successfully;
	  *	ARGUMENT_ERROR if the command was found, but it's arguments
	  *	were invalid; COMMAND_NOT_FOUND if the command was not found;
	  */
	virtual ParseCmd parseCmd(const string &cmd, string &msg) {
		msg.assign(string("Cannot parse: ") + cmd);
		return COMMAND_NOT_FOUND; }
	/** Parse variable requests.
	  * @param[in] name the name of the variable.
	  * @param[in,out] value a string to hold the value of the variable.
	  * @param[in] value_len the length of value.
	  * @returns
	  *	- VARIABLE_NOT_FOUND the name was not found
	  *	- STRING_RETURNED the variable string is in value[]
	  *	- COMMAND_TRUE the variable was a method that returned true
	  *	- COMMAND_FALSE the variable was a method that returned false
	  *	- FOREACH_MORE there are more objects in the array
	  *	- FOREACH_NO_MORE there are no more objects in the array
	  */
	virtual ParseVar parseVar(const string &name, string &value){
		value.assign(name + "%s not found");
		return VARIABLE_NOT_FOUND; }

    protected:
	CommandParser() { }
};

#endif
