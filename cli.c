/******************************************************************************
 * @file    cli.c
 *          CLI (Command Line Interface) function.
 *
 * @author  Nick Yang
 * @date    2018/04/23
 * @version V0.2
 *****************************************************************************/

#ifndef CLI_C_
#define CLI_C_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cli.h"

//General Print
#define CLI_PRINT(msg, args...)  \
    do {\
        printf(msg, ##args);\
    } while (0)

//Error Message output, with RED color.
#define CLI_ERROR(msg, args...)  \
    do {\
        printf("\e[31m"msg"\e[0m", ##args);\
    } while (0)

//Check null pointer and return failure with a simple error message.
#define CHECK_NULL_PTR(ptr) \
    do {\
        if (ptr == NULL) \
        {\
            CLI_ERROR("ERROR: NULL pointer="#ptr"<%s:%d>\n", __FILE__, __LINE__);\
            return CLI_FAILURE;\
        }\
    }while(0)

//Check function return = CLI_SUCCESS, otherwise jump to exit label with a error message.
#define CHECK_FUNC_EXIT(func) \
    do {\
        CLI_RET ret = func;\
        if (CLI_SUCCESS != ret)\
        {\
            CLI_ERROR("ERROR: Return=[%d] "#func"<%s:%d>\n", ret, __FILE__, __LINE__);\
            goto exit;\
        }\
    } while (0)

//Get a integer value from text.
CLI_RET cli_getInt(char *string, int *data_ptr)
{
    CHECK_NULL_PTR(string);
    CHECK_NULL_PTR(data_ptr);
    char *s = NULL;
    int buf = 0;

    buf = (int) strtol(string, &s, 0);

    if (*s != 0)
    {
        return CLI_FAILURE;
    }

    *data_ptr = buf;
    return CLI_SUCCESS;
}

/*!@brief Print option list with help text.
 *
 * @param options   Option list
 * @return          CLI_SUCCESS or CLI_FAILURE of the process
 */
CLI_RET cli_printOptionHelp(Cli_OptionTypeDef options[])
{
    int i = 0;
    while (options[i].OptType != OPT_END)
    {
        if (options[i].OptType == OPT_COMMENT)
        {
            CLI_PRINT("%s:\n", options[i].HelpText);
        }
        else
        {
            CLI_PRINT("\t-%-4c--%-10s:%s\n", options[i].ShortName, options[i].LongName, options[i].HelpText);
        }
        i++;
    }
    return CLI_SUCCESS;
}

CLI_RET cli_printCommandHelp(Cli_CommandTypeDef commands[])
{
    int i = 0;
    CLI_PRINT("%-8s%s\n", "help", "Show the command list");
    while ((commands[i].CommandName != NULL) && (commands[i].FuncCallBack != NULL))
    {
        CLI_PRINT("%-8s%s\n", commands[i].CommandName, commands[i].HelpText);
        i++;
    }

    return CLI_SUCCESS;
}

/*!@brief Get data from according to an option
 *
 * @param argc  Arguments count
 * @param argv  Arguments vector
 * @param option Option to handle the data
 * @return
 */
int Cli_parseData(int argc, char *argv[], Cli_OptionTypeDef option)
{
    switch (option.OptType)
    {
    case OPT_INT:
    {
        int arg_idx = 0;
        int data_idx = 0;
        int *d = (int *) option.ValuePtr;

        for (arg_idx = 0; arg_idx < argc; arg_idx++)
        {
            if ((argv[arg_idx][0] == '-') || (data_idx >= option.ValueCountMax))
            {
                break;
            }
            if (CLI_FAILURE == cli_getInt(argv[arg_idx], d++))
            {
                CLI_ERROR("ERROR: Can't get integer value from [%s]\n", argv[arg_idx]);
                return -1;
            }
            data_idx++;
        }

        if (arg_idx < option.ValueCountMin)
        {
            CLI_ERROR("ERROR:Not enough data, get[%d], required[%d].\n", arg_idx, option.ValueCountMin);
            return -1;
        }

        return arg_idx;
    }
    case OPT_STRING:
    {
        if (argv[0] == NULL)
        {
            CLI_ERROR("ERROR: NULL data for type String!\n");
            return 0;
        }

        char *d = (char*) option.ValuePtr; //Convert pointer type to char *

        strcpy(d++, argv[0]);

        return 1;                   //Convert 1x data string for String type
    }
    case OPT_BOOL:
    {
        _Bool *d = (_Bool*) option.ValuePtr;
        *d = 1;

        return 0;                   //No data needed for Boolean type
    }
    case OPT_COMMENT:
    {
        //No operation for comment only options.
        break;
    }
    default:
    {
        CLI_ERROR("ERROR: Invalid data type of [%d]\n", option.OptType);
        break;
    }
    }
    return 0;
}

/*!@brief handle long options.
 *
 * @param arg_name      pointer to long option name args, e.g. "--test"
 * @param arg_data      pointer to data args, e.g. "0x32"
 * @param options       option list.
 * @return
 */
int Cli_parseOption(int argc, char *argv[], Cli_OptionTypeDef options[])
{
    int i = 0;
    int c = 0;

    while (options[i].OptType != OPT_END)
    {
        if (argv[0][1] == '-') //Long option
        {
            if (options[i].LongName != NULL)
            {
                if (strcmp(&argv[0][2], options[i].LongName) == 0)
                {
                    break;
                }
            }
        }
        else //Short option
        {
            if (options[i].ShortName != 0)
            {
                if (argv[0][1] == options[i].ShortName)
                {
                    break;
                }
            }
        }
        i++;
    }

    // Special type
    if (options[i].OptType == OPT_HELP)
    {
        cli_printOptionHelp(options);
        return 0;
    }

    // Convert arg_data
    c = Cli_parseData(--argc, ++argv, options[i]);
    return c;

    CLI_ERROR("ERROR: Unknown Option [%s]\n", argv[0]);
    return 0;
}

/*!@brief Get a command string. Could route this function to UART if needed.
 *
 * @param string    Pointer to string buffer.
 * @return
 */

/*!@brief Convert a string to an Args list
 *        "This is a -help" -> "This" "is" "a" "-help"
 *
 * @param string    Command line string, e.g. "This is a help"
 * @param argc      Arguments count Output, e.g. 4
 * @param args      Arguments string Output, e.g. "This" "is" "a" "help"
 * @return  CLI_SUCEESS or CLI_FAILURE of the process.
 */
CLI_RET Cli_parseString(char *string, int *argc, char *args[])
{
    CHECK_NULL_PTR(string);
    CHECK_NULL_PTR(argc);
    CHECK_NULL_PTR(args);

    int args_idx = 0;
    int i = 0;
    int len = strlen(string);
    char quote_flag = 0;    //In quote mark""
    char str_flag = 0;        //In str

    for (i = 0; i < len; i++)
    {
        switch (string[i])
        {
        case '\t':
        case ' ':
        case '\r':
        case '\n':
        {
            if (quote_flag == 0)
            {
                string[i] = 0;
                str_flag = 0;
            }
            break;
        }
        case '"':
        {
            quote_flag = !quote_flag;
            string[i] = 0;
            break;
        }
        default:
        {
            if (str_flag == 0)
            {
                args[args_idx++] = &string[i];
            }
            str_flag = 1;
            break;
        }
        }
    }

    *argc = args_idx;
    return CLI_SUCCESS;
}

int Cli_runCommand(int argc, char *args[], Cli_CommandTypeDef commands[])
{
    if (argc == 0)
    {
        //No Commands to run
        return CLI_SUCCESS;
    }
    CHECK_NULL_PTR(args);
    CHECK_NULL_PTR(commands);

    int i = 0;

    //Loop check CommandName and run command.
    while ((commands[i].CommandName != NULL) && (commands[i].FuncCallBack != NULL))
    {
        if (strcmp(commands[i].CommandName, args[0]) == 0)
        {
            return commands[i].FuncCallBack(argc - 1, ++args);
        }
        i++;
    }

    //"help" is a built-in command to show the Command list.
    if (strcmp("help", args[0]) == 0)
    {
        return cli_printCommandHelp(commands);
    }

    //Unknown commands!
    CLI_ERROR("ERROR: Unknown Command of [%s], try [help].\n", args[0]);
    return CLI_FAILURE;
}

/*!@brief Parse the arguments with given options.
 *
 * @param argc      Argument count
 * @param args      Argument string
 * @param options   Argument options list
 * @return          The number of un-used Argument count.
 */
int Cli_parseArgs(int argc, char *args[], Cli_OptionTypeDef options[])
{
    int i;
    int unused_argc = 0;
    char *unused_args[CLI_ARG_COUNT_MAX] =
    { 0 };

    // Loop check arguments, jump to long/short/other type args handler.
    for (i = 0; i < argc; i++)
    {
        if (args[i][0] == '-')
        {
            int ret = Cli_parseOption(argc - i, &args[i], options);
            if (ret < 0)
            {
                CLI_ERROR("ERROR: Failed to handle option [%s]\n", args[i]);
                return ret;
            }
            i += ret;
        }
        else
        {
            //Store un-used args.
            unused_args[unused_argc++] = args[i];
        }
    }

    // If there is a Callback on OPT_END, use it to handle the un-used args.
    for (i = 0;; i++)
    {
        if ((options[i].OptType == OPT_END) && (options[i].CallBack != NULL))
        {
            return options[i].CallBack(unused_argc, unused_args);
        }
    }

    //There's no defined call back at OPT_END, will pass back the unused args.
    for (i = 0; i < unused_argc; i++)
    {
        //args[i] = unused_args[i];
    }
    return unused_argc;
}

#endif /* CLI_C_ */
