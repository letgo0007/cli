/*
 * cli.c
 *
 *  Created on: Mar 13, 2018
 *      Author: nickyang
 */

#ifndef CLI_C_
#define CLI_C_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"

//Check null pointer and return failure with a simple error message.
#define CHECK_NULL_PTR(ptr) \
    do {\
        if (ptr == NULL) \
        {\
            printf("\033[31mERROR: NULL pointer="#ptr"<%s:%d>\033[0m\n", __FILE__, __LINE__);\
            return CLI_FAILURE;\
        }\
    }while(0)

//Check function return = CLI_SUCCESS, otherwise jump to exit label with a error message.
#define CHECK_FUNC_EXIT(func) \
    do {\
        CLI_RET ret = func;\
        if (CLI_SUCCESS != ret)\
        {\
            printf("\033[31mERROR: Return=[%d] "#func"<%s:%d>\033[0m\n", ret, __FILE__, __LINE__);\
            goto exit;\
        }\
    } while (0)

/*!@brief Print option list with help text.
 *
 * @param options   Option list
 * @return          CLI_SUCCESS or CLI_FAILURE of the process
 */
CLI_RET cli_help(stCliOption options[])
{
    int i = 0;
    while (options[i].OptType != OPT_END)
    {
        if (options[i].OptType == OPT_COMMENT)
        {
            printf("%s:\n", options[i].HelpText);
        }
        else
        {
            printf("\t-%-4c--%-10s:%s\n", options[i].ShortName, options[i].LongName, options[i].HelpText);
        }
        i++;
    }
    return CLI_SUCCESS;
}

/*!@brief Convert to text string to value.
 *
 * @param string    Text string, e.g. "0x123"
 * @param data_ptr  Pointer to store the data
 * @param type      Data convert type
 * @return          The number of data_ptr used.
 */
int cli_getData(char *string, void *data_ptr, OPT_TYPE type)
{
    switch (type)
    {
    case OPT_INT:
    {
        if (string == NULL)
        {
            printf("\033[31mERROR: NULL data for type Integer!\033[0m\n");
            return 0;
        }

        char *s = NULL;             //Remaining string pointer
        int *d = (int*) data_ptr;   //Convert pointer type to integer

        *d = (int) strtol(string, &s, 0);
        if (*s != 0)
        {
            goto error;
        }

        return 1;                   //Convert 1x data string for Integer type
    }
    case OPT_STRING:
    {
        if (string == NULL)
        {
            printf("\033[31mERROR: NULL data for type String!\033[0m\n");
            return 0;
        }

        char *d = (char*) data_ptr; //Convert pointer type to char *

        strcpy(d++, string);

        return 1;                   //Convert 1x data string for String type
    }
    case OPT_BOOL:
    {
        char *d = (char*) data_ptr;
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
        error: printf("\033[31mERROR:Invalid data args of [%s]\n\033[0m", string);
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
int cli_handleLongOpt(char *arg_name, char *arg_data, stCliOption options[])
{
    int i = 0;
    int c = 0;

    while (options[i].OptType != OPT_END)
    {
        if (options[i].LongName != NULL)
        {
            // Compare arg_name with text except of "--"
            if (strcmp(&arg_name[2], options[i].LongName) == 0)
            {
                // Special type
                if (options[i].OptType == OPT_HELP)
                {
                    cli_help(options);
                    return 0;
                }

                // Convert arg_data
                c = cli_getData(arg_data, options[i].PtrValue, options[i].OptType);
                return c;
            }
        }
        i++;
    }

    printf("\033[31mERROR: Not Supported Long Option [%s]\n\033[0m", arg_name);
    return 0;
}

int cli_handleShortOpt(char *arg_name, char *arg_data, stCliOption options[])
{
    int i = 0;
    int c = 0;

    while (options[i].OptType != OPT_END)
    {
        if (options[i].ShortName != 0)
        {
            // Compare arg_name with text except of "-"
            if (arg_name[1] == options[i].ShortName)
            {
                // Special type
                if (options[i].OptType == OPT_HELP)
                {
                    cli_help(options);
                    return 0;
                }

                // Convert arg_data
                c = cli_getData(arg_data, options[i].PtrValue, options[i].OptType);
                return c;
            }
        }
        i++;
    }

    printf("\033[31mERROR: Not Supported Short Option [-%c]\n\033[0m", arg_name[1]);
    return 0;
}

/*!@brief Get a command string. Could route this function to UART if needed.
 *
 * @param string    Pointer to string buffer.
 * @return
 */
CLI_RET Cli_getString(char *string)
{
    fgets(string, 100, stdin);
    return CLI_SUCCESS;
}

/*!@brief Convert a string to an Args list
 *        "This is a -help" -> "This" "is" "a" "-help"
 *
 * @param string    Command line string, e.g. "This is a help"
 * @param argc      Arguments count Output, e.g. 4
 * @param args      Arguments string Output, e.g. "This" "is" "a" "help"
 * @return  CLI_SUCEESS or CLI_FAILURE of the process.
 */
CLI_RET Cli_getArgsFromString(char *string, int *argc, char *args[])
{
    CHECK_NULL_PTR(string);
    CHECK_NULL_PTR(argc);
    CHECK_NULL_PTR(args);

    char *token = NULL;
    int i = 0;

    //Get all the tokens from string.
    token = strtok(string, " \n");
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " \n");
    }

    //return args count
    *argc = i;

    return CLI_SUCCESS;
}

/*!@brief Parse the arguments with given options.
 *
 * @param argc      Argument count
 * @param args      Argument string
 * @param options   Argument options
 * @return          The number of un-used Argument count.
 */
int Cli_parseArgs(int argc, char *args[], stCliOption options[])
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
            if (args[i][1] == '-')
            {

                i += cli_handleLongOpt(args[i], args[i + 1], options);
            }
            else
            {

                i += cli_handleShortOpt(args[i], args[i + 1], options);
            }
        }
        else
        {
            //Store un-used args.
            unused_args[unused_argc] = args[i];
            unused_argc++;
        }
    }

    // Call back to handle un-used args.
    for (i = 0; i < argc; i++)
    {
        if ((options[i].OptType == OPT_END) && (options[i].CallBack != NULL))
        {
            options[i].CallBack(unused_argc, unused_args);
        }
    }

    //Pass un-used args back;
    for (i = 0; i < unused_argc; i++)
    {
        args[i] = unused_args[i];
    }
    return unused_argc;
}

#endif /* CLI_C_ */
