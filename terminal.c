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
#include "sys/time.h"

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
        int IntData;
        char StringData[TERM_STRING_BUF_SIZE];
        _Bool BoolData;
    } Tempdata;

    memset(&Tempdata, 0, sizeof(Tempdata));

    //Build the option list for main
    stCliOption MainOpt[] =
    {
    { OPT_COMMENT, 0, NULL, "Basic Options", NULL },
    { OPT_HELP, 'h', "help", "Show help hints", NULL },
    { OPT_INT, 'i', "int", "Get a Integer value", (void*) &Tempdata.IntData },
    { OPT_STRING, 's', "string", "Get a String value", (void*) Tempdata.StringData },
    { OPT_BOOL, 'b', "bool", "Get a Boolean value", (void*) &Tempdata.BoolData },
    { OPT_END, 0, NULL, NULL, NULL, print_args } };

    //Run Arguments parse using MainOpt
    CLI_parseArgs(argc, args, MainOpt);

    //Print Result
    printf("\nResult: Int[%d] String[%s] Bool[%d]\n", Tempdata.IntData, Tempdata.StringData, Tempdata.BoolData);

    return 0;
}

/*!@brief Handler for command "time". Print Unix time stamp.
 */
int Command_time(int argc, char *args[])
{
    time_t timer;
    struct tm *tblock;
    timer = time(NULL);
    tblock = localtime(&timer);

    char buf[32];

    strftime(buf, sizeof(buf), "[%Y-%m-%d-%H:%M:%S]", tblock);
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
    while (argc--)
    {
        printf("%s ", args[i++]);
    }
    return 0;
}

/*!@brief Build the command list.
 *
 */
stCliCommand MainCmd_V1[] =
{
{ "test", Command_test, "Run a argument parse example." },
{ "time", Command_time, "Get current time stamp" },
{ "quit", Command_quit, "Quit the process" },
{ "version", Command_ver, "Show Command version" },
{ "echo", Command_echo, "Echo back command" },
{ NULL, NULL } };

/*!@brief return Terminal prompt string.
 * @return
 */
char *Terminal_prompt(void)
{
#if 1
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
        CLI_convertStrToArgs(sbuf, &argcount, argbuf);
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

