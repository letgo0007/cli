/******************************************************************************
 * @file    cli.h
 *          CLI (Command Line Interface) function.
 *
 * @author  Nick Yang
 * @date    2018/03/15
 * @version V0.1
 *****************************************************************************/

#ifndef CLI_H_
#define CLI_H_

#include <stdint.h>

#define CLI_ARG_COUNT_MAX       16          //!< Number of Args supported.
#define CLI_LINE_END_CHAR       '\n'        //!< Character as line end.
#define CLI_WHITE_SPACE_CHAR    " \t\n\r"   //!< Characters as args seperater

//!@enum    CLI_RET
//!         Return value enum for general Cli function calls.
typedef enum CLI_RET
{
    CLI_SUCCESS = 0,            //!< API success
    CLI_FAILURE = -1,           //!< API fail
} CLI_RET;

//!@enum    OPT_TYPE
//!         Options working type enum.
typedef enum OPT_TYPE
{
    // Non-data options
    OPT_END = 0x00,             //!< End of a command list.
    OPT_HELP,                   //!< Help command that will show all the help text.
    OPT_COMMENT,                //!< Help Text only

    // Basic Data options
    OPT_INT,                    //!< Get 1x integer for this option
    OPT_STRING,                 //!< Get 1x string for this option
    OPT_BOOL,                   //!< Get 1x boolean for this option

    // Extended Data options, not finish yet.
    OPT_MULTI_INT,              //!< Get multiple integer for this option
    OPT_MULTI_STRING,           //!< Get multiple string for this option
    OPT_BIT,                    //!< Set 1 bit of value for this option
    OPT_CALLBACK,               //!< Call a certain call back

} OPT_TYPE;

//!@typedef CliCallBack
//!         CLI call back function call syntax.
typedef int CliCallBack(int argc, char **args);

//!@typedef stCliOption
//!         Structure of a typical Command options.
typedef struct stCliOption
{
    OPT_TYPE OptType;           //!< Option type
    const char ShortName;       //!< Option short name, e.g. 't'
    const char *LongName;       //!< Option long name, e.g. "test"
    const char *HelpText;       //!< Option help text, e.g. "Run the test"
    void *ValuePtr;             //!< Pointer to store option value
    CliCallBack *CallBack;      //!< Function call back
    int ValueCount;             //!< Data amount for multiple data options
} stCliOption;

//!@typedef stCliCommand
//!         Structure of a Cli command.
typedef struct stCliCommand
{
    const char *CommandName;    //!< Command Name
    CliCallBack *FuncCallBack;  //!< Function call back
    const char *HelpText;       //!< Text to describe the function.

} stCliCommand;

CLI_RET CLI_convertStrToArgs(char *string, int *argc, char *args[]);

int CLI_excuteCommand(int argc, char *args[], stCliCommand command[]);

int CLI_parseArgs(int argc, char *args[], stCliOption options[]);

#endif /* CLI_H_ */
