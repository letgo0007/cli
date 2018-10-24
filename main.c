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

#include "cli_simple.h"

extern int builtin_test(int argc, char **args);

int main(int argc, char *args[])
{
    Cli_Register("new1", "new command resister.", &builtin_test);
    Cli_Register("new2", "new command resister.", &builtin_test);
    Cli_Unregister("test");

    while (1)
    {
        static char sbuf[256] =
        { 0 };
        int scount;

        //Start a Mini-Terminal
        scount = Simple_IO_gets(sbuf);
        if (scount > 1)
        {

            Cli_RunString(sbuf);

            memset(sbuf, 0, scount);

            printf("%s", TERM_PROMPT_CHAR);
        }
        else if (scount == 1)
        {
            printf("%s", TERM_PROMPT_CHAR);
        }

        usleep(1000);
    }

    return 0;
}
