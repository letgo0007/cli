/******************************************************************************
 * @file    main.c
 *          The main function entrance.
 *
 * @author  Nick Yang
 * @date    2018/03/15
 * @version V0.1
 *****************************************************************************/

#include "stdio.h"
#include "unistd.h"

#include "cli.h"
#include "terminal.h"

int main(int argc, char *args[])
{
    //Run initial commands
    CLI_excuteCommand(argc - 1, ++args, MainCmd_V1);

    while (1)
    {
        Terminal_run(MainCmd_V1);
        usleep(1000);
    }

    return 0;
}
