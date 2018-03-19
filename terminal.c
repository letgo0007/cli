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
#include "terminal.h"

#ifdef __APPLE__
#if TER_GETS_NON_BLOCK
#include <unistd.h>
#include <fcntl.h>
#endif

int osx_getc(FILE *fp)
{
    char c = fgetc(fp);
    return c;
}

/*!@brief Get a string from a MacOS Terminal (stdin stream)
 *
 * @param string[out]   Pointer to target to store data read from keyboard.
 * @retval < int >      Number of data that is read. The string must end with '\n'.
 *         < 0 >        There is no data, or there's no line end character '\n'.
 */
int osx_gets(char *dest_str, FILE *fp)
{
#if TER_GETS_NON_BLOCK
    //Change STDIO into Non Blocking mode.
    static _Bool initflag = 0;
    if (initflag == 0)
    {
        int old_fl;
        old_fl = fcntl(STDIN_FILENO, F_GETFL);              //Get original STDIN status flag
        fcntl(STDIN_FILENO, F_SETFL, old_fl | O_NONBLOCK);  //Add NON-Blocking flag to STDIN
        initflag = 1;
        printf("STDIN set to non-blocking mode\n>");
    }

    //Build static buffers for string.
    static char sbuf[TER_STRING_BUF_SIZE] =
    { 0 };
    static int idx = 0;
    char c;

    //Loop get 1x char from stdin and check.
    do
    {
        c = osx_getc(fp);
        switch (c)
        {
        case '\xff': //EOF
        {
            break;
        }
        case CLI_LINE_END_CHAR: //End of a line
        {
            sbuf[idx++] = c;
            strcpy(dest_str, sbuf);
            memset(sbuf, 0, sizeof(sbuf));

            int ret = idx;
            idx = 0;
            return ret;                 //return copy data count
        }
        default: //Reject white spaces, need special handle.
        {
            sbuf[idx++] = c;
            break;
        }
        }
    } while (c != '\xff');

    return 0;
#else
    //Blocking mode STDIN;
    fgets(dest_str, 255, stdin);
    return strlen(dest_str);
#endif
}

#endif//__APPLE__

/*!@brief Get a single char from a MCU UART buffer.
 *        Assume it's a ring buffer of size 256.
 *
 * @param src_str[in]   Pointer to the MCU buffer.
 * @retval < char >     A single char of value.
 *         < 0xFF >     No new ch
 */
char mcu_getc(char src_str[])
{
    static int idx = 0;
    char val = src_str[idx];

    if (val == 0)
    {
        return ('\xFF');    //return EOF when no data
    }
    else
    {
#if TER_LOOP_BACK_EN
        printf("%c[%d]", val, val);   //Loop back function.
#endif
        src_str[idx] = 0;    //Clear buf and move index to next.
        idx++;

        //!>@note   Here assume your MCU is using a ring buffer to store received data
        //!         and the size is 256.
        if (idx > TER_STRING_BUF_SIZE)
        {
            idx = 0;
        }
        return (val);
    }
}

int mcu_gets(char *dest_str, char *src_str)
{
    //Build static buffers for string.
    static char sbuf[TER_STRING_BUF_SIZE] =
    { 0 };
    static int idx = 0;
    char c;

    //Loop get 1x char from stdin and check.
    do
    {
        c = mcu_getc(src_str);
        switch (c)
        {
        case '\xff': //EOF
        {
            break;
        }
        case '\b': //Back space
        {
            if (idx > 0)
            {
                sbuf[idx--] = 0;
            }
            break;
        }
        case CLI_LINE_END_CHAR:         //End of a line
        {
            sbuf[idx++] = c;

            strcpy(dest_str, sbuf);     //Copy out whole line.
            memset(sbuf, 0, idx + 1);   //Clear buffer

            int ret = idx;
            idx = 0;
            return ret;                 //return copy data count
        }
        default: //Reject white spaces, need special handle.
        {
            sbuf[idx++] = c;
            break;
        }
        }
    } while (c != '\xff');

    return 0;
}

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
        char StringData[TER_STRING_BUF_SIZE];
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
    time_t tm;
    time(&tm);                              //Get unix time stamp
    printf("%s", asctime(gmtime(&tm)));     //Convert time stamp to a print format.
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

int Terminal_gets(char *string)
{
#ifdef __APPLE__                //!< Usage on MacOs
    return osx_gets(string, stdin);
#endif
#ifdef STM32                    //!< Usage on STM32 MCU
    //!@note Point this buffer to your UART RX buffer.
    static char teststring[] = "test -i 12345678\b\b\n test -h\n  help \n quit\n";
    return mcu_gets(string, teststring);
#endif
}

/* Example of a Mini-Terminal */
int Terminal_run(stCliCommand cmdlist[])
{
    static char sbuf[TER_STRING_BUF_SIZE] =
    { 0 };
    int scount;

    //Start a Mini-Terminal
    scount = Terminal_gets(sbuf);
    if (scount > 1)
    {
        char *argbuf[16] =
        { 0 };
        int argcount = 0;
        CLI_convertStrToArgs(sbuf, &argcount, argbuf);
        CLI_excuteCommand(argcount, argbuf, cmdlist);
        memset(sbuf, 0, scount);
        memset(argbuf, 0, sizeof(char*) * argcount);
        argcount = 0;
        printf(">");
    }
    else if (scount == 1)
    {
        printf(">");
    }

    return 0;
}

