/******************************************************************************
 * @file    main.c
 *          The main function entrance.
 *
 * @author  Nick Yang
 * @date    2018/03/15
 * @version V0.1
 *****************************************************************************/

#include "stdio.h"
#include "string.h"

#include "cli.h"
#include "cmdlist.h"

/* Example of a Mini-Terminal */
int Terminal(stCliCommand cmdlist[])
{
    //Build & clean buffer
    char sbuf[256] =
    { 0 };
    char *argbuf[16] =
    { 0 };
    int argcount = 0;

    //Start a Mini-Terminal

    Cli_getCommand(sbuf);
    if (strlen(sbuf) > 0)
    {
        Cli_convertStrToArgs(sbuf, &argcount, argbuf);
        Cli_excuteCommand(argcount, argbuf, cmdlist);
        printf(">");
    }

    return 0;
}

int main(int argc, char *args[])
{
    //Run initial commands
    Cli_excuteCommand(argc - 1, ++args, MainCmd_V1);

    while (1)
    {
        Terminal(MainCmd_V1);
    }

    return 0;
}
