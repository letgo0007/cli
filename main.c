/*
 * main.c
 *
 *  Created on: Mar 13, 2018
 *      Author: nickyang
 */
#include "cli.h"
#include "stdio.h"
#include "string.h"

int Unused_Args_CallBack(int argc, char **args)
{
    //Example of a call back to handle un-used args.
    //Pass un-used args back;
    int i;
    printf("Un-used argc = [%d]\nUn-used args = ", argc);
    for (i = 0; i < argc; i++)
    {
        printf("%s ", args[i]);
    }
    printf("\n");
    return 0;
}

int main(int argc, char *args[])
{
    //It's Recommended to build a temperory struct to store result.
    struct DataStruct
    {
        int IntData;
        char StringData[256];
        char BoolData;
    } Tempdata;

    //Build the option list for main
    stCliOption MainOpt[] =
    {
    { OPT_COMMENT, 0, NULL, "Basic Options", NULL },
    { OPT_HELP, 'h', "help", "Show help hints", NULL },
    { OPT_INT, 'i', "int", "Get a Integer value", (void*) &Tempdata.IntData },
    { OPT_STRING, 's', "string", "Get a String value", (void*) Tempdata.StringData },
    { OPT_BOOL, 'b', "bool", "Get a Boolean value", (void*) &Tempdata.BoolData },
    { OPT_END, 0, NULL, NULL, NULL, Unused_Args_CallBack } };

    //Run Arguments parse using MainOpt
    Cli_parseArgs(argc, args, MainOpt);

    while (1)
    {

        char sbuf[256] =
        { 0 };
        char *argbuf[16] =
        { 0 };
        int count = 0;

        //Clear data buffer.
        memset(&Tempdata, 0, sizeof(Tempdata));

        //Get a command from stdin
        printf("Please in put a string:\n");
        Cli_getString(sbuf);
        printf("Get String: %s", sbuf);

        //Parse the string to args
        Cli_getArgsFromString(sbuf, &count, argbuf);

        printf("String after split: %s", sbuf);

        //Do Args Prase
        Cli_parseArgs(count, (char **) argbuf, MainOpt);

        //Print result
        printf("\nResult: Int[%d] String[%s] Bool[%d]\n", Tempdata.IntData, Tempdata.StringData, Tempdata.BoolData);
    }

    return 0;
}
