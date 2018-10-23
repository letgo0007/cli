/******************************************************************************
 * @file    cli.h
 *          CLI (Command Line Interface) function.
 *
 * @author  Nick Yang
 * @date    2018/4/23
 * @version V0.2
 *****************************************************************************/

#ifndef CLI_H_
#define CLI_H_

#include <stdint.h>

#define CLI_ARG_COUNT_MAX       16          //!< Number of Args supported.

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
    OPT_INT,                    //!< Get integer for this option
    OPT_STRING,                 //!< Get string for this option
    OPT_BOOL,                   //!< Get boolean for this option

    // Extended Data options, not finish yet.
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
    int ValueCountMin;          //!< Minimum data amount needed for this option
    int ValueCountMax;          //!< Maximum data amount needed for this option
} Cli_OptionTypeDef;

//!@typedef stCliCommand
//!         Structure of a Cli command.
typedef struct stCliCommand
{
    const char *CommandName;    //!< Command Name
    CliCallBack *FuncCallBack;  //!< Function call back
    const char *HelpText;       //!< Text to describe the function.

} Cli_CommandTypeDef;

CLI_RET Cli_parseString(char *string, int *argc, char *args[]);

int Cli_runCommand(int argc, char *args[], Cli_CommandTypeDef command[]);

int Cli_parseArgs(int argc, char *args[], Cli_OptionTypeDef options[]);

#endif /* CLI_H_ */
