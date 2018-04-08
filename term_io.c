#include <stdio.h>
#include <string.h>

#include "term_io.h"
#include "terminal.h"

#if TERM_IO_OS == TERM_OS_OSX
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#endif

#define GET_ARRAY_INDEX(old_idx, move, size)        (((unsigned int)old_idx + move) % (unsigned int)size)

stTermHandle gTermHandle =
{
{ 0 } };

#if TERM_IO_OS == TERM_OS_OSX
/*!@brief   Set STDIN in OSX to non-blocking mode.
 * @retval  [0] Success
 * @retval  [-1] Fail
 */
int osx_initTerm()
{
#if TERM_IO_MODE == TERM_MODE_NONBLOCK
    struct termios new, old;

    //Disable Echo function for STDIN
    int flag;
    if (tcgetattr(STDIN_FILENO, &old) == -1)
    {
        return (-1);
    }
    new = old;
    new.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);

    //Set NONBLOCK mode for STDIN
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new) == -1)
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &old);
        return (-1);
    }
    flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag) == -1)
    {
        return (-1);
    }

    printf("STDIN set to non-blocking mode\n%s", TERM_PROMPT_CHAR);
#else
    //STDIN is blocking mode by default. (Block till you press "ENTER")
#endif
    return 0;
}

int osx_getc(FILE *fp)
{
    char c = fgetc(fp);
    return c;
}

#endif

#if TERM_IO_OS == TERM_OS_STM32
static char uartterm_rx_buf[TERM_STRING_BUF_SIZE] =
{   0};

void stm32_initTerm(char *BufPtr, int BufSize)
{
    //Step1: Link the buffer to UART RX
    /**
     * HAL_UART_Receive_IT(&huart2, BufPtr, BufSize);
     */

    //Step2: Link the UART Rx complete call back to Term_IO_init() to enable automatic ring buffer.
    /**
     * void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
     * {
     *    if (huart == huart2)
     *    {
     *        Term_IO_init();
     *    }
     * }
     *
     */
}

/*!@brief Get a single char from a MCU UART buffer.
 *        Assume it's a ring buffer of size 256.
 *
 * @param src_str[in]   Pointer to the MCU buffer.
 * @retval < char >     A single char of value.
 *         < 0xFF >     No new ch
 */
char stm32_getc(char src_str[])
{
#if TERM_IO_MODE == TERM_MODE_NONBLOCK
    static int idx = 0;
    char val = src_str[idx];

    if (val == 0)
    {
        return ('\xFF');    //return EOF when no data
    }
    else
    {
        src_str[idx] = 0;    //Clear buf and move index to next.
        idx++;

        //!>@note   Here assume your MCU is using a ring buffer to store received data
        //!         and the size is 256.
        if (idx > TERM_STRING_BUF_SIZE)
        {
            idx = 0;
        }
        return (val);
    }
#endif

#if TERM_IO_MODE == TERM_MODE_BLOCK
    char c = 0xff;
    while(HAL_OK != HAL_UART_Receive(huart2,&c,1,10000));
    return c;
#endif

}
#endif

int term_history_push(stTermHandle *TermHandle, int depth)
{
#if TERM_HISTORY_DEPTH
    //Copy string to history
    strcpy(TermHandle->history[TermHandle->his_push_idx], TermHandle->string);

    //Set new push/pull index, Calculate next history index.
    TermHandle->his_pull_idx = TermHandle->his_push_idx;
    TermHandle->his_push_idx = GET_ARRAY_INDEX(TermHandle->his_push_idx, depth, TERM_HISTORY_DEPTH);

    memset(TermHandle->history[TermHandle->his_push_idx], 0, TERM_STRING_BUF_SIZE);
#endif
    return 0;
}

int term_history_pull(stTermHandle *TermHandle, int depth)
{
#if TERM_HISTORY_DEPTH
    //Caculate next pull index
    int new_pull_idx = GET_ARRAY_INDEX(TermHandle->his_pull_idx, depth, TERM_HISTORY_DEPTH);

    //Down Arrow, move before print
    if (depth > 0)
    {
        //Reach the end of history, clear out string.
        if (TermHandle->history[new_pull_idx][0] == 0)
        {
            memset(TermHandle->string, 0, TERM_STRING_BUF_SIZE);
            printf("%s\r%s", TERM_ERASE_LINE_START, TERM_PROMPT_CHAR);
            return 0;
        }
        //Move pull index
        TermHandle->his_pull_idx = new_pull_idx;
    }

    //Copy history info and print the new line
    if (TermHandle->history[TermHandle->his_pull_idx][0] != 0)
    {
        //Copy string from history
        strcpy(TermHandle->string, TermHandle->history[TermHandle->his_pull_idx]);
        TermHandle->index = strlen(TermHandle->string);

        //Print the new string.
        printf("%s\r%s%s", TERM_ERASE_LINE_START, TERM_PROMPT_CHAR, TermHandle->string);
    }

    //Up Arrow, print before move
    if (depth < 0)
    {
        //Reach the end of history, do nothing.
        if (TermHandle->history[new_pull_idx][0] == 0)
        {
            return 0;
        }
        //Move to next pull index
        TermHandle->his_pull_idx = new_pull_idx;
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
        //Pull history previous
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
#if TERM_IO_OS == TERM_OS_OSX
    return osx_initTerm();
#endif

#if TERM_IO_OS == TERM_OS_STM32
    return stm32_initTerm(uartterm_rx_buf, TERM_STRING_BUF_SIZE);
#endif
}

char Term_IO_getc(void)
{
#if TERM_IO_OS == TERM_OS_OSX
    return osx_getc(stdin);
#endif

#if TERM_IO_OS == TERM_OS_STM32
    return stm32_getc(uartterm_rx_buf);
#endif
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
        case CLI_LINE_END_CHAR: //End of a line
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

