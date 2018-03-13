/*
 * main.c
 *
 *  Created on: Mar 13, 2018
 *      Author: nickyang
 */
#include "cli.h"

int main(int argc, char *args[])
{
    //Build the structure of data.
    struct DataStruct {
        int IntData;
        char StringData[256];
        char BoolData;
    };
    struct DataStruct Tempdata = {0};

    //Build the option list for main
    stCliOption MainOpt[] =
    {
            {OPT_INT,'i',"int","Get a Integer value", (void*)&Tempdata.IntData},
            {OPT_STRING,'s',"String","Get a String value", (void*)Tempdata.StringData },
            {OPT_BOOL,'b',"bool","Get a Boolean value", (void*)&Tempdata.BoolData },
    };

    //Run Arguments prase using MainOpt
    Cli_praseArgs(argc-1, ++args, MainOpt);

    return 0;
}
