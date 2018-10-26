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

char *StringBuf = NULL;                     //Command String buffer
unsigned int StringIdx = 0;                 //Command string buffer char index.

char **HistoryBuf = NULL;                   //History pointer buffer
unsigned int HistoryQueueHead = 0;          //History Queue Head
unsigned int HistoryQueueTail = 0;          //History Queue Tail
unsigned int HistoryPullDepth = 0;          //History pull depth, must < CLI_HISTORY_MAX_DEPTH
unsigned int HistoryMemUsage = 0;           //History total memory usage.

CliCommand_TypeDef *CliCommandList = NULL;    //CLI commands list

/*! Porting APIs.
 *
 */

extern void cli_sleep(int us);
void* cli_malloc(size_t size);
void cli_free(void* ptr);
extern int cli_port_init(void);
extern void cli_port_deinit(void);
extern int cli_port_getc(void);

int InsertChar(char *string, char c, int idx)
{
    //right shift buffer from end to insert point
    for (int i = strlen(string); i > idx; i--)
    {
        string[i] = string[i - 1];
    }

    //insert value
    string[idx] = c;

    return 0;
}

int DeleteChar(char *string, int idx)
{
    //left shift buffer from index to end
    for (int i = idx; i < strlen(string) + 1; i++)
    {
        string[i - 1] = string[i];
    }

    return 0;
}

void PrintNewLine(char *string, int pos)
{
    //Erase terminal line, print new buffer string and Move cursor
    printf("%s\r%s%s", ANSI_ERASE_LINE, CLI_PROMPT_CHAR, string);
    printf("\e[%luG", pos + strlen(CLI_PROMPT_CHAR) + 1);
}

void History_Clear(void)
{
    for (int i = 0; i < HISTORY_DEPTH; i++)
    {
        if (HistoryBuf[i] != NULL)
        {
            cli_free(HistoryBuf[i]);
            HistoryBuf[i] = NULL;
        }
    }

    HistoryQueueHead = 0;
    HistoryQueueTail = 0;
    HistoryPullDepth = 0;
    HistoryMemUsage = 0;
}

int History_GetDepth(void)
{
    return HistoryQueueHead - HistoryQueueTail;
}

int History_GetMemUsage(void)
{
    return HistoryMemUsage;
}

/*!@brief Push a string to history queue head.
 *
 */
int History_PushToHead(char *string)
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
    while ((History_GetDepth() >= HISTORY_DEPTH) || (History_GetMemUsage() >= HISTORY_MEM_SIZE))
    {
        //Release from Queue Tail
        int i = HistoryQueueTail % HISTORY_DEPTH;

        if (HistoryBuf[i] != NULL)
        {
            HistoryMemUsage -= strlen(HistoryBuf[i]) + 1;
            cli_free(HistoryBuf[i]);
            HistoryBuf[i] = NULL;
        }

        HistoryQueueTail++;
    }

    return 0;
}

/*!@brief Pull a string from history buffer at certain depth.
 *
 * @param depth
 * @return
 */
int History_PullFromDepth(int depth)
{
    //Calculate history position
    unsigned int pull_idx =
            (depth >= HistoryQueueHead - HistoryQueueTail) ? HistoryQueueTail : HistoryQueueHead - depth;

    if (HistoryBuf[pull_idx % HISTORY_DEPTH] != NULL)
    {
        //Pull out history to string buffer
        memset(StringBuf, 0, CLI_STR_BUF_SIZE);
        strcpy(StringBuf, HistoryBuf[pull_idx % HISTORY_DEPTH]);
        StringIdx = strlen(StringBuf);

        //Print new line on console
        PrintNewLine(StringBuf, StringIdx);
    }

    return 0;
}

int Esc_CheckEscSequence(char c)
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

        if (strcmp(EscBuf, "\e[A") == 0)    //Up Arrow
        {
            if (HistoryPullDepth < History_GetDepth())
            {
                HistoryPullDepth++;
            }
            History_PullFromDepth(HistoryPullDepth);
        }
        else if (strcmp(EscBuf, "\e[B") == 0)    //Down Arrow
        {
            if (HistoryPullDepth > 0)
            {
                HistoryPullDepth--;
            }
            History_PullFromDepth(HistoryPullDepth);
        }
        else if (strcmp(EscBuf, "\e[C") == 0)    //Right arrow
        {
            if (StringBuf[StringIdx] != 0)
            {
                StringIdx++;
                printf("%s", ANSI_CURSOR_RIGHT);
            }
        }
        else if (strcmp(EscBuf, "\e[D") == 0)    //Left arrow
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

int builtin_history(int argc, char **argv)
{
    const char *helptext = "history \n"
            "\t-d --dump  Dump command history.\n"
            "\t-c --clear Clear command history.\n";

    printf("Index  Address    Command\n");

    for (int i = HistoryQueueTail; i < HistoryQueueHead; i++)
    {
        int j = i % HISTORY_DEPTH;
        printf("%-6d 0x%08X %s\n", i, (int) HistoryBuf[j], (HistoryBuf[j] == NULL) ? "NULL" : HistoryBuf[j]);
    }
    return 0;
}

int builtin_test(int argc, char **args)
{
    printf("Argc = %d\n", argc);
    for (int i = 0; i < argc; i++)
    {
        printf("Args[%d] = %s\n", i, args[i] == NULL ? "NULL" : args[i]);
    }

    return 0;
}

int builtin_repeat(int argc, char **args)
{
    const char *helptext = "repeat [num] [command]\n";
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

int Cli_GetLine(char **ptr)
{
    char c = 0;

    do
    {
        //Get 1 char and check
        c = cli_port_getc();

        //Handle characters
        switch (c)
        {
        case '\x0':    //NULL
        case '\xff':    //EOF
        {
            break;
        }
        case '\x7f':    //Delete
        case '\b':    //Backspace
        {
            if (StringIdx > 0)
            {
                //Delete 1 byte from buffer.
                DeleteChar(StringBuf, StringIdx);
                StringIdx--;
                //Print New line
                PrintNewLine(StringBuf, StringIdx);

            }
            break;
        }
        case '\r':    //End of a line
        case '\n':    //End of a line
        {
            //Push to history without \'n'
            if (StringIdx > 0)
            {
                History_PushToHead(StringBuf);
            }

            //Echo back
            strcat(StringBuf, "\n");
            printf("\n");

            // Return pointer and length
            StringIdx = 0;
            HistoryPullDepth = 0;
            int len = strlen(StringBuf);
            *ptr = StringBuf;

            return len;    //return copy data count
        }
        default:
        {
            //The letters after a ESC is terminal control characters
            if (Esc_CheckEscSequence(c) == 0)
            {
                return 0;
            }
            else
            {
                //Insert 1 byte to buffer
                if (strlen(StringBuf) < CLI_STR_BUF_SIZE - 2)
                {
                    InsertChar(StringBuf, c, StringIdx);
                    StringIdx++;

                    //Loop Back function
                    if (1)
                    {
                        if (StringBuf[StringIdx] == 0)
                        {
                            //Loop back 1 byte if it's the end of a line.
                            printf("%c", c);
                        }
                        else
                        {
                            PrintNewLine(StringBuf, StringIdx);
                        }
                    }
                }
            }
            break;
        }
        }
    } while (c != '\xff');

    return 0;
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
        // Find a empty slot to save the command.
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
                printf("\n%s\n", ret ? "FAIL" : "OK");
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
    char **cmds = cli_malloc(sizeof(char*) * 16);
    char *cmdtoken = NULL;

    cmdtoken = strtok(cmd, ";");
    while ((cmdtoken != NULL) && (cmdc < 16))
    {
        cmds[cmdc] = cmdtoken;
        cmdc++;
        cmdtoken = strtok(NULL, ";");
    }

    // Run every commands
    for (int i = 0; i < cmdc; i++)
    {
        int argc = 0;
        char **args = cli_malloc(sizeof(char*) * 32);
        char *token = NULL;

        token = strtok(cmds[i], " \t\r\n");
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

    cli_free(cmds);

    return 0;

}

int Cli_Init(void)
{
    // Clear operation buffers
    StringIdx = 0;
    StringBuf = cli_malloc(sizeof(char) * CLI_STR_BUF_SIZE);
    HistoryBuf = cli_malloc(sizeof(char*) * HISTORY_DEPTH);
    CliCommandList = cli_malloc(sizeof(CliCommand_TypeDef) * CLI_CMD_LIST_SIZE);

    History_Clear();

    // Register built-in commands.
    Cli_Register("help", "Show command & prompt.", &builtin_help);
    Cli_Register("history", "Show command history", &builtin_history);
    Cli_Register("test", "Run a command parse example", &builtin_test);
    Cli_Register("repeat", "Repeat run a command.", &builtin_repeat);
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

    History_Clear();

    StringIdx = 0;
    cli_free(StringBuf);
    cli_free(HistoryBuf);
    cli_free(CliCommandList);

    return 0;
}

int Cli_Run(void)
{
    char *sbuf = NULL;
    int scount = 0;

    scount = Cli_GetLine(&sbuf);

    if (scount > 1)
    {
        Cli_RunByString(sbuf);
        memset(sbuf, 0, scount);
        printf("%s", CLI_PROMPT_CHAR);
    }
    else if (scount == 1)
    {
        printf("%s", CLI_PROMPT_CHAR);
        memset(sbuf, 0, scount);
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

