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

    int test[8] =
    { 1, 2, 3, 4, 5, 6, 7, 8 };

    for (int i = 0; i < 256; i++)
    {
        printf("0x%X\t", test[i % 8]);
    }

    //Run initial commands
    Cli_runCommand(argc - 1, ++args, MainCmd_V1);

    while (1)
    {
        Terminal_run(MainCmd_V1);
        usleep(1000);
    }

    return 0;
}
