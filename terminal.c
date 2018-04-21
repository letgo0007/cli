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
#include "term_io.h"
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
int Command_test(int argc, char *args[])
{
    //It's Recommended to build a temperory struct to store result.
    struct DataStruct
    {
        int IntData[16];
        char StringData[TERM_STRING_BUF_SIZE];
        _Bool BoolData;
    } Tempdata;

    memset(&Tempdata, 0, sizeof(Tempdata));

    //Build the option list for main
    stCliOption MainOpt[] =
    {
    { OPT_COMMENT, 0, NULL, "Basic Options", NULL, NULL, 0 },
    { OPT_HELP, 'h', "help", "Show help hints", NULL, NULL, 0 },
    { OPT_INT, 'i', "int", "Get a Integer value", (void*) Tempdata.IntData, NULL, 16 },
    { OPT_STRING, 's', "string", "Get a String value", (void*) Tempdata.StringData, NULL, 0 },
    { OPT_BOOL, 'b', "bool", "Get a Boolean value", (void*) &Tempdata.BoolData, NULL, 0 },
    { OPT_END, 0, NULL, NULL, NULL, print_args } };

    //Run Arguments parse using MainOpt
    CLI_parseArgs(argc, args, MainOpt);

    //Print Result
    printf("\nResult: Int[%d][%d] String[%s] Bool[%d]\n", Tempdata.IntData[0], Tempdata.IntData[1], Tempdata.StringData,
            Tempdata.BoolData);

    return 0;
}

int Command_sleep(int argc, char *args[])
{
    float sec = strtof(args[0], NULL);
    usleep(1000000 * sec);
    return 0;
}

int Command_repeat(int argc, char *args[])
{
    int count = strtol(args[0], NULL, 0);
    int i = 0;

    for (i = 0; i < count; i++)
    {
        char cmd_buf[TERM_STRING_BUF_SIZE] =
        { 0 };
        char *argbuf[TERM_TOKEN_AMOUNT] =
        { 0 };
        int argcount = 0;

        strcpy(cmd_buf, args[1]);

        printf("%sRepeat [%d/%d] of [%s]\n%s", TERM_BOLD, i + 1, count, cmd_buf, TERM_RESET);
        CLI_StrToArgs(cmd_buf, &argcount, argbuf);
        CLI_excuteCommand(argcount, argbuf, MainCmd_V1);
    }

    return 0;
}

int Command_time(int argc, char *args[])
{
    struct timeb t_start;
    struct timeb t_end;

    //Get start/stop time stampe and run the command.
    ftime(&t_start);
    int ret = CLI_excuteCommand(argc, args, MainCmd_V1);
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
int Command_ver(int argc, char *args[])
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

int Command_echo(int argc, char *args[])
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

int Command_history(int argc, char *args[])
{
    int i = 0;

    printf("Command History:\n[Index][Command]\n");
    for (i = 0; i < TERM_HISTORY_DEPTH; i++)
    {
        printf("[%5d][%s]%s%s\n", i,                                // History index
                gTermHandle.history[i],                             // History string
                (i == gTermHandle.his_push_idx) ? "<-Push" : "",    // Push index
                (i == gTermHandle.his_pull_idx) ? "<-Pull" : "");   // Pull index
    }
    return 0;
}

/*!@brief Build the command list.
 *
 */
stCliCommand MainCmd_V1[] =
{
{ "test", Command_test, "Run a argument parse example." },
{ "time", Command_time, "Get the execute time of a command." },
{ "sleep", Command_sleep, "Sleep process, unit in second." },
{ "repeat", Command_repeat, "Repeat run a command." },
{ "date", Command_date, "Get current time stamp" },
{ "quit", Command_quit, "Quit the process" },
{ "version", Command_ver, "Show Command version" },
{ "echo", Command_echo, "Echo back command" },
{ "history", Command_history, "Show command history" },
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
    return Term_IO_gets(string, &gTermHandle);
}

/* Example of a Mini-Terminal */
int Terminal_run(stCliCommand cmdlist[])
{
    static char sbuf[TERM_STRING_BUF_SIZE] =
    { 0 };
    int scount;

    //Start a Mini-Terminal
    scount = Terminal_gets(sbuf);
    if (scount > 1)
    {
        char *argbuf[TERM_TOKEN_AMOUNT] =
        { 0 };
        int argcount = 0;
        CLI_StrToArgs(sbuf, &argcount, argbuf);
        CLI_excuteCommand(argcount, argbuf, cmdlist);
        memset(sbuf, 0, scount);
        memset(argbuf, 0, sizeof(char*) * argcount);
        argcount = 0;
        printf("%s", TERM_PROMPT_CHAR);
    }
    else if (scount == 1)
    {
        printf("%s", TERM_PROMPT_CHAR);
    }

    return 0;
}

