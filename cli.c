/*
 * cli_simple.c
 *
 *  Created on: Oct 23, 2018
 *      Author: nickyang
 */

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#include <cli.h>

char *StringBuf = NULL;                 //Command String buffer pointer
unsigned int StringIdx = 0;             //Command string index

char **HistoryBuf = NULL;               //History pointer buffer pointer
unsigned int HistoryQueueHead = 0;      //History queue head
unsigned int HistoryQueueTail = 0;      //History queue tail
unsigned int HistoryPullDepth = 0;      //History pull depth
unsigned int HistoryMemUsage = 0;       //History total memory usage

CliCommand_TypeDef *CliCommandList = NULL;    //CLI commands list pointer

/*! Porting APIs.
 *
 */

extern void cli_sleep(int us);
extern void* cli_malloc(size_t size);
extern void cli_free(void* ptr);
extern int cli_port_init(void);
extern void cli_port_deinit(void);
extern int cli_port_getc(void);

/*!@brief Insert a char to a position of a string.
 *
 * @param string    Pointer to the old string
 * @param c         Char to insert
 * @param pos       Position to insert
 * @return          Pointer to the new string or NULL when it fails.
 */
char* insert_char(char *string, char c, int pos)
{
    if ((string == NULL) || (pos > strlen(string)))
    {
        return NULL;
    }

    //right shift buffer from end to insert point
    for (int i = strlen(string); i > pos; i--)
    {
        string[i] = string[i - 1];
    }

    //insert value
    string[pos] = c;

    return string;
}

/*!@brief Insert a char to a position of a string.
 *
 * @param string    Pointer to the old string
 * @param pos       Position to delete
 * @return          Pointer to the new string or NULL when it fails.
 */
char* delete_char(char *string, int pos)
{
    if ((string == NULL) || (pos > strlen(string)))
    {
        return NULL;
    }

    //left shift buffer from index to end
    for (int i = pos; i < strlen(string) + 1; i++)
    {
        string[i - 1] = string[i];
    }

    return string;
}

/*!@brief Print a new line on terminal and set the curse to a position.
 *
 * @param string    String to print.
 * @param pos       Position to put the curse
 */
void print_newline(char *string, int pos)
{
    //Erase terminal line, print new buffer string and Move cursor
    printf("%s\r%s%s", ANSI_ERASE_LINE, CLI_PROMPT_CHAR, string);
    printf("\e[%luG", pos + strlen(CLI_PROMPT_CHAR) + 1);
}

/*!@brief Clear history buffer & heap.
 *
 */
void history_clear(void)
{
    // Try free string heap if used.
    for (int i = 0; i < HISTORY_DEPTH; i++)
    {
        if (HistoryBuf[i] != NULL)
        {
            cli_free(HistoryBuf[i]);
            HistoryBuf[i] = NULL;
        }
    }

    // Reset index
    HistoryQueueHead = 0;
    HistoryQueueTail = 0;
    HistoryPullDepth = 0;
    HistoryMemUsage = 0;
}

/*!@brief Get the number of commands stored in history heap.
 *
 * @return
 */
int history_getdepth(void)
{
    return HistoryQueueHead - HistoryQueueTail;
}

/*!@brief Get the number of bytes heap memory used to store history string.
 *
 * @return
 */
int history_getmem(void)
{
    return HistoryMemUsage;
}

/*!@brief Push a string to history queue head.
 *
 * @param string    String to put to history
 * @return Pointer to where the history is stored.
 */
char* history_push(char *string)
{
    //Request memory & copy command
    unsigned int len = strlen(string) + 1;
    char *ptr = cli_malloc(len);
    memcpy(ptr, string, len);

    //Save new history queue pointer & queue head.
    HistoryBuf[HistoryQueueHead % HISTORY_DEPTH] = ptr;
    HistoryQueueHead++;
    HistoryMemUsage += len;

    //Release History buffer if number or memory usage out of limit
    while ((history_getdepth() >= HISTORY_DEPTH) || (history_getmem() >= HISTORY_MEM_SIZE))
    {
        //Release from Queue Tail
        int idx = HistoryQueueTail % HISTORY_DEPTH;

        if (HistoryBuf[idx] != NULL)
        {
            HistoryMemUsage -= strlen(HistoryBuf[idx]) + 1;
            cli_free(HistoryBuf[idx]);
            HistoryBuf[idx] = NULL;
        }

        HistoryQueueTail++;
    }

    return ptr;
}

/*!@brief Pull a string from history buffer at certain depth.
 *
 * @param depth     The depth of history to pull.
 *                  1 means you are pulling the newest, larger value means older.
 * @return          Pointer to pulled history buffer or NULL for failure.
 */
char* history_pull(int depth)
{
    //Calculate where to pull the history.
    unsigned int pull_idx = (HistoryQueueHead - depth) % HISTORY_DEPTH;

    if (HistoryBuf[pull_idx] != NULL)
    {
        //Pull out history to string buffer
        memset(StringBuf, 0, CLI_STR_BUF_SIZE);
        strcpy(StringBuf, HistoryBuf[pull_idx]);
        StringIdx = strlen(StringBuf);

        //Print new line on console
        print_newline(StringBuf, StringIdx);
    }

    return HistoryBuf[pull_idx];
}

/*!@brief Check if a string is part of ANSI escape sequence.
 *        Refer to <https://en.wikipedia.org/wiki/ANSI_escape_code>
 *        Loop put the character from a string to this function.
 *        This function give terminal the ability to response to some multi-byte Keyboard keys.
 *
 * @param  c    Character to check.
 * @retval 0    The character is part of escape sequence.
 * @retval c    The character is not part
 */
int esc_handler(char c)
{
    static char EscBuf[8] =
    { 0 };
    static int EscIdx = 0;
    static char EscFlag = 0;

    //Start of ESC flow control
    if (c == '\e')
    {
        EscFlag = 1;
        EscIdx = 0;
        memset(EscBuf, 0, 8);
    }

    //Return the character unchanged if not Escape sequence.
    if (EscFlag == 0)
    {
        return c;
    }
    else
    {
        //Put character to Escape sequence buffer
        EscBuf[EscIdx++] = c;

        if (strcmp(EscBuf, "\e[A") == 0)            //!<Up Arrow
        {
            if (HistoryPullDepth < history_getdepth())
            {
                HistoryPullDepth++;
            }
            history_pull(HistoryPullDepth);
        }
        else if (strcmp(EscBuf, "\e[B") == 0)       //!<Down Arrow
        {
            if (HistoryPullDepth > 0)
            {
                HistoryPullDepth--;
            }
            history_pull(HistoryPullDepth);
        }
        else if (strcmp(EscBuf, "\e[C") == 0)       //!<Right arrow
        {
            if (StringBuf[StringIdx] != 0)
            {
                StringIdx++;
                printf("%s", ANSI_CURSOR_RIGHT);
            }
        }
        else if (strcmp(EscBuf, "\e[D") == 0)       //!<Left arrow
        {
            if (StringIdx > 0)
            {
                StringIdx--;
                printf("%s", ANSI_CURSOR_LEFT);
            }
        }

        // Escape Sequence is ended by a Letter, clear buffer and flag for next new operation.
        if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
        {
            EscFlag = 0;
            memset(EscBuf, 0, 8);
            EscIdx = 0;
        }

        return 0;
    }

    return 0;
}

/*!@brief Built-in command of "help"
 *
 */
int builtin_help(int argc, char **argv)
{
    for (int i = 0; i < CLI_CMD_LIST_SIZE; i++)
    {
        if ((CliCommandList[i].Name != NULL) && (CliCommandList[i].Prompt != NULL))
        {
            printf("%-12s%s\n", CliCommandList[i].Name, CliCommandList[i].Prompt);
        }
    }
    return 0;
}

/*!@brief Built-in command of "history"
 *
 */
int builtin_history(int argc, char **args)
{
    const char *helptext = "history usage:\n"
            "\t-d --dump  Dump command history.\n"
            "\t-c --clear Clear command history.\n"
            "\t-h --help  Show this help text.\n";

    if ((argc < 2) || (args[argc - 1] == NULL))
    {
        printf("%s", helptext);
        return -1;
    }

    if ((strcmp("-d", args[1]) == 0) || (strcmp("--dump", args[1]) == 0))
    {
        printf("History dump:\n");
        printf("Index  Address    Command\n");
        printf("-------------------------\n");
        for (int i = HistoryQueueTail; i < HistoryQueueHead; i++)
        {
            int j = i % HISTORY_DEPTH;
            printf("%-6d 0x%08X %s\n", i, (int) HistoryBuf[j], (HistoryBuf[j] == NULL) ? "NULL" : HistoryBuf[j]);
        }
    }
    else if ((strcmp("-c", args[1]) == 0) || (strcmp("--clear", args[1]) == 0))
    {
        printf("History clear!\n");
        history_clear();
    }
    else if ((strcmp("-h", args[1]) == 0) || (strcmp("--help", args[1]) == 0))
    {
        printf("%s", helptext);
    }
    else
    {
        printf("ERROR: invalid option of [%s]\n", args[1]);
    }

    return 0;
}

/*!@brief Built-in command of "test"
 *
 */
int builtin_test(int argc, char **args)
{
    printf("Argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("Args[%d] = %s\n", i, args[i] == NULL ? "NULL" : args[i]);
    }

    return 0;
}

/*!@brief Built-in command of "repeat"
 *
 */
int builtin_repeat(int argc, char **args)
{
    const char *helptext = "usage: repeat [num] [command]\n";

    if ((argc < 3) || (args == NULL))
    {
        printf("%s", helptext);
        return -1;
    }

    unsigned int count = strtol(args[1], NULL, 0);

    for (unsigned int i = 1; i <= count; i++)
    {
        printf("%sRepeat %d/%d:%s\n", ANSI_BOLD, i, count, ANSI_RESET);
        int ret = Cli_RunByArgs(argc - 2, args + 2);
        if (ret != 0)
        {
            return ret;
        }
    }

    return 0;
}

/*!@brief Built-in command of "sleep"
 *
 */
int builtin_sleep(int argc, char **args)
{
    const char *helptext = "usage: sleep [ms]\n";

    if ((argc <= 1) || (args[1] == NULL))
    {
        printf("%s", helptext);
        return -1;
    }

    unsigned int ms = strtol(args[1], NULL, 0);
    cli_sleep(ms);

    return 0;
}

char Cli_getopt(int argc, char **args, CliOption_TypeDef options[])
{
    static int op_argc = 0;
    static char **op_args = NULL;
    static int op_index = 0;

    if ((argc != 0) && (args != NULL))
    {
        op_argc = argc;
        op_args = args;
        op_index = 0;
    }

    if (op_index < op_argc)
    {
        if(op_args[op_index][0] == '-')
        {
            if(op_args[op_index][1] == '-') //Long options with "--"
            {
                ;
            }
            else //Short Options with "-"
            {
                ;
            }
        }
        else //parameters with out "--"
        {
            ;
        }

        op_index ++;
    }

    return 0;
}

/*!@brief Get a line for CLI.
 *        This function will check input from cli_port_getc() function.
 *        Put them to buffer until get a new line "\n".
 *
 * @return Pointer to the line or NULL for no line is get.
 */
char* Cli_GetLine(void)
{
    char c = 0;

    do
    {
        //Get 1 char and check
        c = cli_port_getc();

        //Handle characters
        switch (c)
        {
        case '\x0':     //NULL
        case '\xff':    //EOF
        {
            break;
        }
        case '\x7f':    //Delete for MacOs keyboard
        case '\b':      //Backspace PC keyboard
        {
            if (StringIdx > 0)
            {
                //Delete 1 byte from buffer.
                delete_char(StringBuf, StringIdx);
                StringIdx--;
                //Print New line
                print_newline(StringBuf, StringIdx);
            }
            break;
        }
        case '\r':      //CR
        case '\n':      //LF
        {
            // Push to history without \'n'
            if (StringIdx > 0)
            {
                history_push(StringBuf);
            }

            // Echo back
            strcat(StringBuf, "\n");
            printf("\n");

            // Return pointer and length
            StringIdx = 0;
            HistoryPullDepth = 0;

            // eturn copy data count
            return StringBuf;
        }
        default:
        {
            //The letters after a ESC is terminal control characters
            if (esc_handler(c) == 0)
            {
                return 0;
            }
            else
            {
                //Insert 1 byte to buffer
                if (strlen(StringBuf) < CLI_STR_BUF_SIZE - 2)
                {
                    insert_char(StringBuf, c, StringIdx);
                    StringIdx++;

                    //Loop back function
                    if (StringBuf[StringIdx] == 0)
                    {

                        printf("%c", c);
                    }
                    else
                    {
                        print_newline(StringBuf, StringIdx);
                    }
                }
            }
            break;
        }
        }
    } while (c != '\xff');

    return NULL;
}

int Cli_Register(const char *name, const char *prompt, int (*func)(int, char **))
{
    if ((name == NULL) || (prompt == NULL) || func == NULL)
    {
        return -1;
    }

    for (int i = 0; i < CLI_CMD_LIST_SIZE; i++)
    {
        // Find a empty slot to save the command.
        if ((CliCommandList[i].Name == NULL) && (CliCommandList[i].Func == NULL))
        {
            CliCommandList[i].Name = name;
            CliCommandList[i].Prompt = prompt;
            CliCommandList[i].Func = func;
            return 0;
        }
    }

    return -1;
}

int Cli_Unregister(const char *name)
{
    for (int i = 0; i < CLI_CMD_LIST_SIZE; i++)
    {
        // Delete the command
        if (strcmp(CliCommandList[i].Name, name) == 0)
        {
            CliCommandList[i].Name = NULL;
            CliCommandList[i].Prompt = NULL;
            CliCommandList[i].Func = NULL;
            return 0;
        }
    }
    return -1;
}

int Cli_RunByArgs(int argc, char **args)
{
    if ((argc == 0) || (args == NULL))
    {
        return -1;
    }

    for (int i = 0; i < CLI_CMD_LIST_SIZE; i++)
    {
        if ((CliCommandList[i].Name != NULL) && (CliCommandList[i].Func != NULL))
        {
            if (strcmp(CliCommandList[i].Name, args[0]) == 0)
            {
                int ret = CliCommandList[i].Func(argc, args);
                printf("%s\n", ret ? "FAIL" : "OK");
                return ret;
            }
        }
    }

    printf("ERROR: Unknown Command of [%s], try [help].\n", args[0]);
    return -1;
}

int Cli_RunByString(char *cmd)
{
    if (cmd == NULL)
    {
        return -1;
    }

    // Find commands Separate by ";"
    int cmdc = 0;
    char **cmdv = cli_malloc(sizeof(char*) * 16);
    char *cmdtoken = NULL;

    cmdtoken = strtok(cmd, ";");
    while ((cmdtoken != NULL) && (cmdc < 16))
    {
        cmdv[cmdc] = cmdtoken;
        cmdc++;
        cmdtoken = strtok(NULL, ";");
    }

    // Run every commands
    for (int i = 0; i < cmdc; i++)
    {
        int argc = 0;
        char **args = cli_malloc(sizeof(char*) * 32);
        char *token = NULL;

        // String to token
        token = strtok(cmdv[i], " \t\r\n");
        while ((token != NULL) && (argc < 32))
        {
            args[argc] = token;
            argc++;
            token = strtok(NULL, " \t\r\n");

        }
        args[argc] = NULL;

        Cli_RunByArgs(argc, args);

        cli_free(args);
    }

    cli_free(cmdv);

    return 0;

}

int Cli_Init(void)
{
    // Clear operation buffers
    StringIdx = 0;
    StringBuf = cli_malloc(sizeof(char) * CLI_STR_BUF_SIZE);
    HistoryBuf = cli_malloc(sizeof(char*) * HISTORY_DEPTH);
    CliCommandList = cli_malloc(sizeof(CliCommand_TypeDef) * CLI_CMD_LIST_SIZE);

    history_clear();

    // Register built-in commands.
    Cli_Register("help", "Show command & prompt.", &builtin_help);
    Cli_Register("history", "Show command history", &builtin_history);
    Cli_Register("test", "Run a command parse example", &builtin_test);
    Cli_Register("repeat", "Repeat run a command.", &builtin_repeat);
    Cli_Register("sleep", "Sleep unit in ms", &builtin_sleep);
    // Initialize IO port
    cli_port_init();

    printf("---------------------------------\n");
    printf("Command Line Interface %s\n", CLI_VERSION);
    printf("Compiler      = %s\n", __VERSION__);
    printf("Date/Time     = %s %s\n", __DATE__, __TIME__);
    printf("Operation Mem = %ld Byte\n",
            sizeof(char) * CLI_STR_BUF_SIZE + sizeof(char*) * HISTORY_DEPTH
                    + sizeof(CliCommand_TypeDef) * CLI_CMD_LIST_SIZE);
    printf("History Mem   = %d Byte\n", HISTORY_MEM_SIZE);
    printf("---------------------------------\n");

    return 0;
}

int Cli_Deinit(void)
{
    cli_port_deinit();

    history_clear();

    StringIdx = 0;
    cli_free(StringBuf);
    cli_free(HistoryBuf);
    cli_free(CliCommandList);

    return 0;
}

int Cli_Run(void)
{
    char *str = Cli_GetLine();

    if (str != NULL)
    {
        if (strlen(str) >= 1)
        {
            Cli_RunByString(str);
        }
        memset(StringBuf, 0, CLI_STR_BUF_SIZE);
        printf("%s", CLI_PROMPT_CHAR);
    }

    return 0;
}

int Cli_Task(void)
{
    Cli_Init();
    while (1)
    {
        Cli_Run();
        cli_sleep(1);
    }
}

