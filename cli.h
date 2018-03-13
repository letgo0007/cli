/*
 * cli.h
 *
 *  Created on: Mar 13, 2018
 *      Author: nickyang
 */

#ifndef CLI_H_
#define CLI_H_

#define CLI_ARG_COUNT_MAX       16
#define CLI_ARGS_SIZE_MAX       16

typedef enum CLI_RET
{
    CLI_SUCCESS = 0,
    CLI_FAILURE = -1,
}CLI_RET;

typedef enum OPT_TYPE
{
    OPT_END = 0x00,         //End of options

    OPT_INT,                //Option value is integer
    OPT_STRING,             //Option value is string
    OPT_BOOL,               //Option value is boolean

    OPT_COMMENT = 0xFF,     //Option value is comments, will print help string only.
} OPT_TYPE;

typedef struct stCliOption
{
    OPT_TYPE OptType;       //Option type
    const char ShortName;   //Option short name, e.g. 't'
    const char *LongName;   //Option long name, e.g. "test"
    const char *HelpText;   //Option help text, e.g. "Run the test"
    void *PtrValue;         //Pointer to store option value
    int DataArgsAmount;     //number of args that is data
} stCliOption;

CLI_RET Cli_getString(char *string);

CLI_RET Cli_getArgsFromString(char *string, int *argc, char args[CLI_ARG_COUNT_MAX][CLI_ARGS_SIZE_MAX]);

CLI_RET Cli_praseArgs(int argc, char *args[], stCliOption options[]);

#endif /* CLI_H_ */
