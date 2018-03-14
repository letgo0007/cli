/*
 * cli.h
 *
 *  Created on: Mar 13, 2018
 *      Author: nickyang
 */

#ifndef CLI_H_
#define CLI_H_

#define CLI_ARG_COUNT_MAX       16      //Number of Args supported.
#define CLI_ARGS_SIZE_MAX       16      //Maximum Length of a single args

typedef enum CLI_RET
{
    CLI_SUCCESS = 0, CLI_FAILURE = -1,
} CLI_RET;

typedef enum OPT_TYPE
{
    OPT_END = 0x00,             //End of options
    OPT_HELP = 0x01,            //Show help text of all options.
    OPT_COMMENT = 0x02,         //Option value is comments, will print help string only.

    OPT_INT = 0x10,             //Option value is integer
    OPT_STRING = 0x11,          //Option value is string
    OPT_BOOL = 0x12,            //Option value is boolean

} OPT_TYPE;

typedef int CliCallBack(int argc, char **args);

typedef struct stCliOption
{
    OPT_TYPE OptType;           //Option type
    const char ShortName;       //Option short name, e.g. 't'
    const char *LongName;       //Option long name, e.g. "test"
    const char *HelpText;       //Option help text, e.g. "Run the test"
    void *PtrValue;             //Pointer to store option value
    CliCallBack *CallBack;      //Function call back
} stCliOption;

typedef struct stCliCommand
{
    const char *CommandName;
    CliCallBack *FuncCallBack;
    const char *HelpText;

} stCliCommand;

CLI_RET Cli_getCommand(char *string);

CLI_RET Cli_convertStrToArgs(char *string, int *argc, char *args[]);

int Cli_excuteCommand(int argc, char *args[], stCliCommand command[]);

int Cli_parseArgs(int argc, char *args[], stCliOption options[]);

#endif /* CLI_H_ */
