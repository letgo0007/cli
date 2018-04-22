#include <stdio.h>
#include <string.h>

#include "term_io.h"
#include "terminal.h"

#if TERM_IO_OS == TERM_OS_OSX
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

stTermHandle gTermHandle =
{
{ 0 } };

int term_get_array_index(int old_idx, int move, int size)
{
    int new_idx = old_idx + move;
    if (new_idx < 0)
    {
        new_idx = size - 1;
    }
    else if (new_idx >= size)
    {
        new_idx = 0;
    }
    return new_idx;
}

int term_history_push(stTermHandle *TermHandle, int depth)
{
#if TERM_HISTORY_DEPTH
    //Copy string to history
    strcpy(TermHandle->history[TermHandle->his_push_idx], TermHandle->string);

    //Set new push/pull index, Calculate next history index.
    TermHandle->his_pull_idx = TermHandle->his_push_idx;
    TermHandle->his_push_idx = term_get_array_index(TermHandle->his_push_idx, depth, TERM_HISTORY_DEPTH);

    memset(TermHandle->history[TermHandle->his_push_idx], 0, TERM_STRING_BUF_SIZE);
#endif
    return 0;
}

int term_history_pull(stTermHandle *TermHandle, int depth)
{
#if TERM_HISTORY_DEPTH
    //Calculate next pull index
    int new_pull_idx = term_get_array_index(TermHandle->his_pull_idx, depth, TERM_HISTORY_DEPTH);

    //**Up Arrow**, Print & Move
    if (depth < 0)
    {
        //Copy history info and print the new line
        if (TermHandle->history[TermHandle->his_pull_idx][0] != 0)
        {
            //Copy string from history
            strcpy(TermHandle->string, TermHandle->history[TermHandle->his_pull_idx]);
            TermHandle->index = strlen(TermHandle->string);

            //Print the new string.
            printf("%s\r%s%s", TERM_ERASE_LINE, TERM_PROMPT_CHAR, TermHandle->string);
        }

        //Reach the end of history, do nothing.
        if (TermHandle->history[new_pull_idx][0] == 0)
        {
            return 0;
        }
        //Move to next pull index
        TermHandle->his_pull_idx = new_pull_idx;
    }
    //**Down Arrow**, Move and print
    else if (depth > 0)
    {
        //Reach the end of history, clear out string.
        if (TermHandle->history[new_pull_idx][0] == 0)
        {
            memset(TermHandle->string, 0, TERM_STRING_BUF_SIZE);
            TermHandle->index = 0;
            printf("%s\r%s", TERM_ERASE_LINE, TERM_PROMPT_CHAR);
            return 0;
        }
        //Move pull index
        TermHandle->his_pull_idx = new_pull_idx;

        //Copy history info and print the new line
        if (TermHandle->history[TermHandle->his_pull_idx][0] != 0)
        {
            //Copy string from history
            strcpy(TermHandle->string, TermHandle->history[TermHandle->his_pull_idx]);
            TermHandle->index = strlen(TermHandle->string);

            //Print the new string.
            printf("%s\r%s%s", TERM_ERASE_LINE, TERM_PROMPT_CHAR, TermHandle->string);
        }
    }
#endif
    return 0;
}

int term_esc_handle(char c, stTermHandle *TermHandle)
{
    static char escbuf[8] =
    { 0 };
    static int escidx = 0;

    if (c == '\e')  //Start of ESC folow control, default length = 2
    {
        TermHandle->esc_flag = 2;
    }
    if (c == '[') //Start of a Cursor control, max length = 4, till get a letter.
    {
        TermHandle->esc_flag += 4;
    }

    escbuf[escidx++] = c;

    if (strcmp(escbuf, "\e[A") == 0)    //Up Arrow
    {
        //Pull previous history
        term_history_pull(TermHandle, -1);
    }
    else if (strcmp(escbuf, "\e[B") == 0) //Down Arrow
    {
        //Pull next history
        term_history_pull(TermHandle, 1);
    }
    else if (strcmp(escbuf, "\e[C") == 0)   //Right arrow
    {

        if (TermHandle->string[TermHandle->index] != 0)
        {
            (TermHandle->index)++;
            printf("%s", TERM_CURSOR_RIGHT);
        }
    }
    else if (strcmp(escbuf, "\e[D") == 0)   //Left arrow
    {
        if (TermHandle->index > 0)
        {
            (TermHandle->index)--;
            printf("%s", TERM_CURSOR_LEFT);
        }
    }

    //A letter is the end of a Cursor control sequence.
    if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
    {
        TermHandle->esc_flag = 0;
        memset(escbuf, 0, 8);
        escidx = 0;
    }

    return 0;

}

/*!@brief Insert a char into a string at certain position.
 *
 * @param string
 * @param c
 * @param idx
 */
int term_insert(char *string, char c, int idx)
{
    int len = strlen(string);

    if (idx > len)
    {
        //can't insert if index out of range.
        return -1;
    }

    //right shift buffer from index to end
    int i = 0;
    for (i = len; i > idx; i--)
    {
        string[i] = string[i - 1];
    }

    //insert value
    string[idx] = c;

    return 0;
}

int term_delete(char *string, int idx)
{
    int len = strlen(string);

    if (idx > len)
    {
        //can't insert if index out of range.
        return -1;
    }

    //left shift buffer from index to end
    int i = 0;
    for (i = idx; i < len + 1; i++)
    {
        string[i - 1] = string[i];
    }

    return 0;

}

int Term_IO_init()
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

    printf("STDIN set to non-blocking mode\n%s", TERM_PROMPT_CHAR);

    return 0;
}

char Term_IO_getc(void)
{
    return getchar();
}

int Term_IO_gets(char *dest_str, stTermHandle *TermHandle)
{
    //Initialize terminal IO
    static int initflag = 1;
    if (initflag)
    {
        Term_IO_init();
        TermHandle->loopback_enable = TERM_LOOP_BACK_EN;
        initflag = 0;
    }

    char c = 0;

    do
    {
        //Get 1 char and check
        c = Term_IO_getc();

        //Handle characters
        switch (c)
        {
        case '\x0':  //NULL
        case '\xff': //EOF
        {
            break;
        }
        case '\e': //ESC
        {
            term_esc_handle(c, TermHandle);
            break;
        }
        case '\x7f': //Delete
        case '\b': //Backspace
        {
            if (TermHandle->index > 0)
            {
                //Delete 1 byte from buffer.
                term_delete(TermHandle->string, TermHandle->index);
                //Erase terminal line and print new buffer string.
                printf("%s\r%s%s", TERM_ERASE_LINE, TERM_PROMPT_CHAR, TermHandle->string);
                //Move cursor
                printf("\e[%luG", TermHandle->index + strlen(TERM_PROMPT_CHAR));
                TermHandle->index--;
            }

            break;
        }
        case '\r': //End of a line
        case '\n': //End of a line
        {
            //Push to history without \'n'
            if (TermHandle->index > 0)
            {
                term_history_push(TermHandle, 1);
            }

            //Copy out string with '\n'
            strcat(TermHandle->string, "\n");
            strcpy(dest_str, TermHandle->string);
            printf("\n");

            //Clear buffer
            int ret = strlen(TermHandle->string);
            memset(TermHandle->string, 0, TERM_STRING_BUF_SIZE);
            TermHandle->index = 0;

            return ret; //return copy data count
        }
        default:
        {
            //The letters after a ESC is terminal control characters
            if (TermHandle->esc_flag > 0)
            {
                term_esc_handle(c, TermHandle);
                TermHandle->esc_flag--;
            }
            else
            {
                //Insert 1 byte to buffer
                term_insert(TermHandle->string, c, TermHandle->index++);

                //Loop Back function
                if (TermHandle->loopback_enable)
                {
                    if (TermHandle->string[TermHandle->index] == 0)
                    {
                        //Loop back 1 byte if it's the end of a line.
                        printf("%c", c);
                    }
                    else
                    {
                        //Erase entire line and print new buffer content
                        printf("%s\r%s%s", TERM_ERASE_LINE, TERM_PROMPT_CHAR, TermHandle->string);
                        //Move cursor
                        printf("\e[%luG", TermHandle->index + strlen(TERM_PROMPT_CHAR) + 1);
                    }

                }
            }
            break;
        }
        }
    } while (c != '\xff');

    return 0;
}

