/******************************************************************************
 * @file    main.c
 *          The main function entrance.
 *
 * @author  Nick Yang
 * @date    2018/11/02
 * @version V1.0
 *****************************************************************************/

#include <cli.h>
#include "stdio.h"
#include "unistd.h"

int main(int argc, char *args[])
{
    Cli_Init();

    while (1)
    {
        Cli_Run();
        usleep(1000);
    }

}
