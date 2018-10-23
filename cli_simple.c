/*
 * cli_simple.c
 *
 *  Created on: Oct 23, 2018
 *      Author: nickyang
 */

#include "stdio.h"
#include "unistd.h"
#include <fcntl.h>
#include <termios.h>

#include "stdint.h"
#include "cli_simple.h"
#include "cli_simple_conf.h"

uint8_t StringBuf[CLI_STRING_BUF_SIZE] =            //Command string buffer
{ 0 };
uint16_t StringIdx = 0;                             //Command string buffer char index.

uint8_t *HistoryBuf[CLI_HISTORY_MAX_DEPTH] =        //History pointer buffer
{ 0 };
uint16_t HistoryQueueHead = 0;                      //History Queue Head
uint16_t HistoryQueueTail = 0;                      //History Queue Tail
uint16_t HistoryPullDepth = 0;                      //History pull depth, must < CLI_HISTORY_MAX_DEPTH
uint32_t HistoryMemUsage = 0;                       //History total memory usage.

int InsertChar(char *string, char c, int idx)
{
    int len = strlen(string);

    if (idx > len)
    {
        return -1;
    }

    //right shift buffer from index to end
    for (int i = len; i > idx; i--)
    {
        string[i] = string[i - 1];
    }

    //insert value
    string[idx] = c;

    return 0;
}

int DeleteChar(char *string, int idx)
{
    int len = strlen(string);

    if (idx > len)
    {
        return -1;
    }

    //left shift buffer from index to end
    for (int i = idx; i < len + 1; i++)
    {
        string[i - 1] = string[i];
    }

    return 0;
}

void PrintNewLineWithCurse(char *string, int pos)
{
    //Erase terminal line, print new buffer string and Move cursor
    printf("%s\r%s%s", TERM_ERASE_LINE, TERM_PROMPT_CHAR, string);
    printf("\e[%luG", pos + strlen(TERM_PROMPT_CHAR) + 1);
}

void PrintHistory()
{
    printf("Index   Address Command\n");

    for (int i = HistoryQueueTail; i < HistoryQueueHead; i++)
    {
        printf("%8d 0x%08X %s\n", i, HistoryBuf[i % CLI_HISTORY_MAX_DEPTH], HistoryBuf[i % CLI_HISTORY_MAX_DEPTH]);
    }
}

int History_GetNum(void)
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
    uint32_t len = strlen(string) + 1;
    uint8_t *ptr = malloc(len);
    memcpy(ptr, string, len);

    //Save new history queue pointer & queue head.
    HistoryBuf[HistoryQueueHead % CLI_HISTORY_MAX_DEPTH] = ptr;
    HistoryQueueHead++;
    HistoryMemUsage += len;

    //Release History buffer if number or memory usage out of limit
    while ((HistoryQueueHead - HistoryQueueTail >= CLI_HISTORY_MAX_DEPTH) || (HistoryMemUsage >= CLI_HISTORY_MAX_MEM))
    {
        //Release from Queue Tail
        HistoryMemUsage -= strlen(HistoryBuf[HistoryQueueTail]) + 1;
        if (HistoryBuf[HistoryQueueTail % CLI_HISTORY_MAX_DEPTH] != NULL)
        {
            free(HistoryBuf[HistoryQueueTail]);
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
int History_PullFromDepth(uint16_t depth)
{
    //Calculate history position
    uint16_t pull_idx = (depth >= HistoryQueueHead - HistoryQueueTail) ? HistoryQueueTail : HistoryQueueHead - depth;

    if (HistoryBuf[pull_idx % CLI_HISTORY_MAX_DEPTH] != NULL)
    {
        //Pull out history to string buffer
        memset(StringBuf, 0, CLI_STRING_BUF_SIZE);
        strcpy(StringBuf, HistoryBuf[pull_idx % CLI_HISTORY_MAX_DEPTH]);
        StringIdx = strlen(StringBuf);

        //Print new line on console
        PrintNewLineWithCurse(StringBuf, StringIdx);
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
            if (HistoryPullDepth < History_GetNum())
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
                printf("%s", TERM_CURSOR_LEFT);
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
    //Initialize terminal IO
    static int initflag = 1;
    if (initflag)
    {
        Simple_IO_init();
        initflag = 0;
    }

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
        case '\b': //Backspace
        {
            if (StringIdx > 0)
            {
                //Delete 1 byte from buffer.
                DeleteChar(StringBuf, StringIdx);
                StringIdx--;
                //Print New line
                PrintNewLineWithCurse(StringBuf, StringIdx);

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
                        PrintNewLineWithCurse(StringBuf, StringIdx);
                    }
                }
            }
            break;
        }
        }
    } while (c != '\xff');

    return 0;
}

