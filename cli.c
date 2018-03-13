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

/*!@brief Get data according to args type.
 *
 * @param arg_data
 * @param data_ptr
 * @param data_count
 * @param type
 * @return
 */
int cli_getData(char *string[], void *data_ptr, int data_count, OPT_TYPE type)
{
    CHECK_NULL_PTR(string);
    CHECK_NULL_PTR(data_ptr);

    switch (type)
    {
    case OPT_INT:
    {
        char *s = NULL;
        int *d = (int*) data_ptr;
        int i = 0;

        for (i = 0; i < data_count; i++)
        {
            CHECK_NULL_PTR(string[i]);
            //Convert string to int
            *d++ = (int) strtol(string[i], &s, 0);
            //Handle string error
            if (*s != 0)
            {
                goto error;
            }
        }

        break;
    }

    case OPT_STRING:
    {
        int i = 0;
        char *d = (char*) data_ptr;
        for (i = 0; i < data_count; i++)
        {
            CHECK_NULL_PTR(string[i]);
            strcpy(d++, string[i]);
        }
        break;
    }

    case OPT_BOOL:
    {
        *(char*) data_ptr = 1;
        break;
    }

    default:
    {
        error: printf("ERROR:Invalid data args of [%s]\n", string[0]);
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
    printf("Jump in Long option\n");
#if 1
    int i = 0;

    while (options[i].OptType != OPT_END)
    {
        printf("OptLongName = [%s] ",options[i].LongName);
        printf("StringName = [%s]\n",&arg_name[2]);
        if (strcmp(&arg_name[2], options[i].LongName) == 0)
        {
            printf("Run cli_getData");
            cli_getData(&arg_data, options[i].PtrValue, options[i].DataArgsAmount, options[i].OptType);
        }
        i++;
    }
#endif
    return 1;
}

int cli_handleShortOpt(char *arg_name, char *arg_data, stCliOption options[])
{
    printf("Jump in Short option\n");
    return 1;
}

CLI_RET Cli_getString(char *string)
{
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
CLI_RET Cli_getArgsFromString(char *string, int *argc, char args[CLI_ARG_COUNT_MAX][CLI_ARGS_SIZE_MAX])
{
    CHECK_NULL_PTR(string);
    CHECK_NULL_PTR(argc);
    CHECK_NULL_PTR(args);

    char *token = NULL;
    char *str_temp = malloc(strlen(string));
    int i = 0;

    //Buffer the string for operation
    strcpy(str_temp, string);

    //Get all the tokens from string.
    token = strtok(str_temp, " \n");
    while (token != NULL)
    {
        strcpy(&args[i++][0], token);
        token = strtok(NULL, " \n");
    }

    //free memory
    free(str_temp);

    //return args count
    *argc = i;

    return CLI_SUCCESS;
}

CLI_RET Cli_praseArgs(int argc, char *args[], stCliOption options[])
{

#if 1
    int value[10] =
    { 0 };
    char argbuf[16][16] =
    {
    { 0 } };
    int count;

    //Test cli_getData
    char *string = "this123213312 is a test";
    cli_getData(&args[1], value, 2, OPT_INT);

    printf("Arg:[%s],value[%d][%d]\n", args[1], value[0], value[1]);

    //Test cli_getArgsFromString
    Cli_getArgsFromString(string, &count, argbuf);

    printf("argc = [%d]\n", count);
    printf("argbuf:[%s][%s][%s][%s]\n", argbuf[0], argbuf[1], argbuf[2], argbuf[3]);
    printf("argbuf:[%s][%s][%s][%s]\n", argbuf[4], argbuf[5], argbuf[6], argbuf[7]);
#endif

    int i;
    int unused_argc = 0;
    char *unused_args[CLI_ARG_COUNT_MAX] =
    { 0 };

    //Loop check arguments, for long/short/other type.
    for (i = 0; i < argc; i++)
    {
        printf("args:[%s]\n", args[i]);
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
            printf("not a option!\n");
        }
    }

    printf("unused argc = [%d], args[%s][%s][%s]\n", unused_argc, unused_args[0], unused_args[1], unused_args[2]);

    return CLI_SUCCESS;
}

#endif /* CLI_C_ */
