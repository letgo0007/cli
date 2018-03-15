/*
 * cli.h
 *
 *  Created on: Mar 13, 2018
 *      Author: nickyang
 */

#ifndef CLI_H_
#define CLI_H_

#include <stdint.h>

#define CLI_ARG_COUNT_MAX       16      //Number of Args supported.

typedef enum CLI_RET
{
    CLI_SUCCESS = 0, CLI_FAILURE = -1,
} CLI_RET;

typedef enum OPT_TYPE
{
    // Non-data options
    OPT_END = 0x00,             //End of options
    OPT_HELP,                   //Show help text of all options.
    OPT_COMMENT,                //Option value is comments, will print help string only.


    // Basic Data options
    OPT_INT,                    //Option value is integer
    OPT_STRING,                 //Option value is string
    OPT_BOOL,                   //Option value is boolean

    // Extended Data options, not finish yet.
    OPT_MULTI_INT,              //Get multiple integer value
    OPT_MULTI_STRING,           //Get Multiple string value
    OPT_BIT,                    //Set a single bit.
    OPT_CALLBACK,               //Run the Call back function only.

} OPT_TYPE;

typedef int CliCallBack(int argc, char **args);

typedef struct stCliOption
{
    OPT_TYPE OptType;           //Option type
    const char ShortName;       //Option short name, e.g. 't'
    const char *LongName;       //Option long name, e.g. "test"
    const char *HelpText;       //Option help text, e.g. "Run the test"
    void *ValuePtr;             //Pointer to store option value
    CliCallBack *CallBack;      //Function call back
    int ValueCount;             //Data amount for multiple data options
} stCliOption;

typedef struct stCliCommand
{
    const char *CommandName;    //Command Name
    CliCallBack *FuncCallBack;  //Function call back
    const char *HelpText;       //Text to describe the function.

} stCliCommand;

CLI_RET Cli_getCommand(char *string);

CLI_RET Cli_convertStrToArgs(char *string, int *argc, char *args[]);

int Cli_excuteCommand(int argc, char *args[], stCliCommand command[]);

int Cli_parseArgs(int argc, char *args[], stCliOption options[]);

#endif /* CLI_H_ */
