/******************************************************************************
 * @file    terminal.c
 *          An example of building a command list for CLI.
 *
 * @author  Nick Yang
 * @date    2018/03/15
 * @version V0.1
 *****************************************************************************/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <sys/time.h>
#include <sys/timeb.h>
#include "unistd.h"

#include "cli.h"
#include "cli_simple.h"
#include "terminal.h"

/*!@brief A simple example of handling un-used args. Just print them.
 *
 * @param argc[in]  Argument Count
 * @param args[in]  Pointer to Argument String
 *
 * @retval 0        Success
 * @retval -1       Failure
 */
int print_args(int argc, char **args)
{
    if ((argc == 0) || (args == 0))
    {
        return -1;
    }

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

/*!@brief Handler for command "test".
 *        An example to get 1x interger value, 1x string value, 1x bool value.
 *
 * @param argc[in]  Argument Count
 * @param args[in]  Pointer to Argument String
 *
 * @retval 0        Success
 * @retval -1       Failure
 */
int cli_test(int argc, char *args[])
{
    //It's Recommended to build a temperory struct to store result.
    struct DataStruct
    {
        int IntData[16];
        char StringData[256];
        _Bool BoolData;
    } Tempdata;

    memset(&Tempdata, 0, sizeof(Tempdata));

    //Build the option list for main
    Cli_OptionTypeDef MainOpt[] =
    {
    { OPT_COMMENT, 0, NULL, "Basic Options" },
    { OPT_HELP, 'h', "help", "Show help hints" },
    { OPT_INT, 'i', "int", "Get a Integer value", (void*) Tempdata.IntData, NULL, 1, 16 },
    { OPT_STRING, 's', "string", "Get a String value", (void*) Tempdata.StringData, NULL },
    { OPT_BOOL, 'b', "bool", "Get a Boolean value", (void*) &Tempdata.BoolData, NULL },
    { OPT_END, 0, NULL, NULL, NULL, print_args } };

    //Run Arguments parse using MainOpt
    Cli_parseArgs(argc, args, MainOpt);

    //Print Result
    printf("\nResult: Int[%d][%d] String[%s] Bool[%d]\n", Tempdata.IntData[0], Tempdata.IntData[1], Tempdata.StringData,
            Tempdata.BoolData);

    return 0;
}

int cli_wait(int argc, char *args[])
{
    float sec = strtof(args[0], NULL);
    usleep(1000000 * sec);
    return 0;
}


int cli_time(int argc, char *args[])
{
    struct timeb t_start;
    struct timeb t_end;

    //Get start/stop time stampe and run the command.
    ftime(&t_start);
    int ret = Cli_runCommand(argc, args, MainCmd_V1);
    ftime(&t_end);

    int dif_ms;
    int dif_s;

    if (t_end.millitm >= t_start.millitm)
    {
        dif_ms = t_end.millitm - t_start.millitm;
        dif_s = t_end.time - t_start.time;
    }
    else
    {
        dif_ms = t_end.millitm - t_start.millitm + 1000;
        dif_s = t_end.time - t_start.time - 1;
    }

    printf("time: %d.%03d s\n", dif_s, dif_ms);
    return ret;
}

/*!@brief Handler for command "time". Print Unix time stamp.
 */
int Command_date(int argc, char *args[])
{
    time_t timer;
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);

    char buf[32];

    strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", tblock);
    printf("%s", buf);
    return 0;
}

/*!@brief Handler for command "time". Print Unix time stamp.
 */
int cli_ver(int argc, char *args[])
{
    printf("Version(compile time): %s %s\n", __DATE__, __TIME__);
    return 0;
}

/*!@brief Handler for command "quit". Quit process.
 */
int Command_quit(int argc, char *args[])
{
    exit(0);
}

int cli_echo(int argc, char *args[])
{
    int i = 0;
    char strbuf[256] =
    { 0 };

    while (argc--)
    {
        strcat(strbuf, args[i++]);
        strcat(strbuf, " ");
    }
    strcat(strbuf, "\n");
    printf("%s", strbuf);
    return 0;
}

extern PrintHistory();
int cli_history(int argc, char *args[])
{
    PrintHistory();
}

/*!@brief Build the command list.
 *
 */
Cli_CommandTypeDef MainCmd_V1[] =
{
{ "test", cli_test, "Run a argument parse example." },
{ "time", cli_time, "Get the execute time of a command." },
{ "sleep", cli_wait, "Sleep process, unit in second." },
{ "date", Command_date, "Get current time stamp" },
{ "quit", Command_quit, "Quit the process" },
{ "version", cli_ver, "Show Command version" },
{ "echo", cli_echo, "Echo back command" },
{ "history", cli_history, "Show command history" },
{ NULL, NULL } };

/*!@brief return Terminal prompt string.
 * @return
 */
char *Terminal_prompt(void)
{
#if 0
    time_t timer;
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);
    static char buf[32];

    strftime(buf, sizeof(buf), "[%H:%M:%S]", tblock);
    return buf;
#else
    static char buf[2] = ">";
    return buf;
#endif

}

int Terminal_gets(char *string)
{
    return Simple_IO_gets(string);
}

/* Example of a Mini-Terminal */
int Terminal_run(Cli_CommandTypeDef cmdlist[])
{
    static char sbuf[256] =
    { 0 };
    int scount;

    //Start a Mini-Terminal
    scount = Terminal_gets(sbuf);
    if (scount > 1)
    {
        char *argbuf[16] =
        { 0 };
        int argcount = 0;
        Cli_parseString(sbuf, &argcount, argbuf);
        Cli_runCommand(argcount, argbuf, cmdlist);

        memset(sbuf, 0, scount);

        printf("%s", TERM_PROMPT_CHAR);
    }
    else if (scount == 1)
    {
        printf("%s", TERM_PROMPT_CHAR);
    }

    return 0;
}

