/*
 * cli_simple.c
 *
 *  Created on: Oct 23, 2018
 *      Author: nickyang
 */

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "stdlib.h"

#include <fcntl.h>
#include <termios.h>
#include <getopt.h>

#include "cli_simple.h"
#include "cli_simple_conf.h"

int builtin_help(int argc, char **argv);
int builtin_history(int argc, char **argv);
int builtin_test(int argc, char **argv);

char StringBuf[CLI_STRING_BUF_SIZE] =
{ 0 };                                                  //Command string buffer
unsigned int StringIdx = 0;                             //Command string buffer char index.

char *HistoryBuf[HISTORY_DEPTH_LIMIT] =
{ 0 };                                                  //History pointer buffer
unsigned int HistoryQueueHead = 0;                      //History Queue Head
unsigned int HistoryQueueTail = 0;                      //History Queue Tail
unsigned int HistoryPullDepth = 0;                      //History pull depth, must < CLI_HISTORY_MAX_DEPTH
unsigned int HistoryMemUsage = 0;                       //History total memory usage.

CliCommand_TypeDef CliCommandList[COMMAND_NUM_LIMIT] =
{
{ "help", "Show command and prompt list", &builtin_help },
{ "history", "Show command history", &builtin_history },
{ "test", "Run a command parse example", &builtin_test }, };

int InsertChar(char *string, char c, int idx)
{
    //right shift buffer from index to end
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
    printf("%s\r%s%s", ANSI_ERASE_LINE, TERM_PROMPT_CHAR, string);
    printf("\e[%luG", pos + strlen(TERM_PROMPT_CHAR) + 1);
}

int History_Clear(void)
{
    for (int i = 0; i < HISTORY_DEPTH_LIMIT; i++)
    {
        if (HistoryBuf[i] != NULL)
        {
            free(HistoryBuf[i]);
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
    char *ptr = malloc(len);
    memcpy(ptr, string, len);

    //Save new history queue pointer & queue head.
    HistoryBuf[HistoryQueueHead % HISTORY_DEPTH_LIMIT] = ptr;
    HistoryQueueHead++;
    HistoryMemUsage += len;

    //Release History buffer if number or memory usage out of limit
    while ((HistoryQueueHead - HistoryQueueTail >= HISTORY_DEPTH_LIMIT) || (HistoryMemUsage >= HISTORY_MEM_LIMIT))
    {
        //Release from Queue Tail
        int i = HistoryQueueTail % HISTORY_DEPTH_LIMIT;

        if (HistoryBuf[i] != NULL)
        {
            HistoryMemUsage -= strlen(HistoryBuf[i]) + 1;
            free(HistoryBuf[i]);
            HistoryBuf[i] = NULL;
        }

        HistoryQueueTail++;
    }

    //Clear out current string buffer
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

    if (HistoryBuf[pull_idx % HISTORY_DEPTH_LIMIT] != NULL)
    {
        //Pull out history to string buffer
        memset(StringBuf, 0, CLI_STRING_BUF_SIZE);
        strcpy(StringBuf, HistoryBuf[pull_idx % HISTORY_DEPTH_LIMIT]);
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
        else if (strcmp(EscBuf, "\e[B") == 0) //Down Arrow
        {
            if (HistoryPullDepth > 0)
            {
                HistoryPullDepth--;
            }
            History_PullFromDepth(HistoryPullDepth);
        }
        else if (strcmp(EscBuf, "\e[C") == 0)   //Right arrow
        {
            if (StringBuf[StringIdx] != 0)
            {
                StringIdx++;
                printf("%s", ANSI_CURSOR_RIGHT);
            }
        }
        else if (strcmp(EscBuf, "\e[D") == 0)   //Left arrow
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

int Simple_IO_init()
{
    struct termios new, old;
    int flag;

    //Disable Echo function for STDIN
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);

    //Set STDIN to Non-blocking mode.
    flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag) == -1)
    {
        return (-1);
    }

    setvbuf(stdout, (char*) NULL, _IOLBF, 256);
    setvbuf(stderr, (char*) NULL, _IONBF, 256);
    setvbuf(stdin, (char*) NULL, _IONBF, 256);

    printf("STDIN set to non-blocking mode\n%s", TERM_PROMPT_CHAR);

    return 0;
}

char Simple_IO_getc(void)
{
    return getchar();
}

int Simple_IO_gets(char *dest_str)
{
    char c = 0;

    do
    {
        //Get 1 char and check
        c = Simple_IO_getc();

        //Handle characters
        switch (c)
        {
        case '\x0':  //NULL
        case '\xff': //EOF
        {
            break;
        }
        case '\x7f': //Delete
        case '\b':   //Backspace
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
        case '\r': //End of a line
        case '\n': //End of a line
        {
            //Push to history without \'n'
            if (StringIdx > 0)
            {
                History_PushToHead(StringBuf);
            }

            //Copy out string with '\n'
            strcat(StringBuf, "\n");
            strcpy(dest_str, StringBuf);
            printf("\n");

            //Clear buffer
            int len = strlen(StringBuf);
            memset(StringBuf, 0, CLI_STRING_BUF_SIZE);
            StringIdx = 0;
            HistoryPullDepth = 0;

            return len; //return copy data count
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
            break;
        }
        }
    } while (c != '\xff');

    return 0;
}

int builtin_help(int argc, char **argv)
{
    for (int i = 0; i < COMMAND_NUM_LIMIT; i++)
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
    printf(" Index Address   Command\n");

    for (int i = HistoryQueueTail; i < HistoryQueueHead; i++)
    {
        int j = i % HISTORY_DEPTH_LIMIT;
        printf("%6d 0x%08X %s\n", i, (int) HistoryBuf[j], (HistoryBuf[j] == NULL) ? "NULL" : HistoryBuf[j]);
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

int Cli_Unregister(const char *name)
{
    for (int i = 0; i < COMMAND_NUM_LIMIT; i++)
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

int Cli_Register(const char *name, const char *prompt, int (*func))
{
    if ((name == NULL) || (prompt == NULL) || func == NULL)
    {
        return -1;
    }

    for (int i = 0; i < COMMAND_NUM_LIMIT; i++)
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

int Cli_Init(void)
{
    Simple_IO_init();
    History_Clear();
    return 0;
}

int Cli_RunArgs(int argc, char **args)
{
    if ((argc == 0) || (args == NULL))
    {
        return -1;
    }

    for (int i = 0; i < COMMAND_NUM_LIMIT; i++)
    {
        if ((CliCommandList[i].Name != NULL) && (CliCommandList[i].Func != NULL))
        {
            if (strcmp(CliCommandList[i].Name, args[0]) == 0)
            {
                return CliCommandList[i].Func(argc, args);
            }
        }
    }

    printf("ERROR: Unknown Command of [%s], try [help].\n", args[0]);
    return -1;
}

int Cli_RunString(char *cmd)
{
    if (cmd == NULL)
    {
        return -1;
    }

    int argc = 0;
    char **args = malloc(sizeof(char*) * 64);
    char *token = NULL;

    token = strtok(cmd, " \t\r\n");
    while (token != NULL)
    {
        args[argc] = token;
        argc++;
        token = strtok(NULL, " \t\r\n");
    }
    args[argc] = NULL;

    Cli_RunArgs(argc, args);

    free(args);

    return 0;

}

