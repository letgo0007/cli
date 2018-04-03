#include "term_history.h"
#include "term_io.h"
#include "terminal.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

stTermBuf TermBuf =
{
{ 0 } };

#if 0
int ascii_esc_handle(char c, char* s, int *i)
{
    static char sbuf[8] =
    {   0};
    static int idx = 0;

    sbuf[idx++] = c;

    if (strcmp(sbuf, "\e[A") == 0)
    {
        printf("\e[2K\r>%s", history[his_read_idx]);
        strcpy(s, history[his_read_idx]);
        *i = strlen(history[his_read_idx]);
        his_read_idx--;
        if (his_read_idx < 0)
        {
            his_read_idx = TER_HISTORY_DEPTH - 1;
        }
        //printf("Get Up Arrow\n");
    }
    else if (strcmp(sbuf, "\e[B") == 0)
    {
        printf("\e[2K\r>%s", history[his_read_idx]);
        strcpy(s, history[his_read_idx]);
        *i = strlen(history[his_read_idx]);
        his_read_idx++;
        if (his_read_idx >= TER_HISTORY_DEPTH)
        {
            his_read_idx = 0;
        }
    }
    else if (strcmp(sbuf, "\e[C") == 0)
    {
        printf("Get Right Arrow\n");
    }
    else if (strcmp(sbuf, "\e[D") == 0)
    {
        printf("Get Left Arrow\n");
    }

    if (idx > 2)
    {
        memset(sbuf, 0, 8);
        idx = 0;
    }
    return 0;
}
#endif

/*!@brief   Set STDIN in OSX to non-blocking mode.
 *
 * @return
 */
int osx_initTerm()
{
#if TERM_IO_MODE == TERM_MODE_NONBLOCK
    struct termios new, old;
    int flag;
    if (tcgetattr(STDIN_FILENO, &old) == -1)
    {
        return (1);
    }

    new = old;
    new.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &new) == -1)
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &old);
        return (1);
    }
    flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag) == -1)
    {
        return (1);
    }

    printf("STDIN set to non-blocking mode\n>");
#endif

    return 0;

}

int osx_getc(FILE *fp)
{
    char c = fgetc(fp);
    return c;
}

void stm32_initTerm(char *BufPtr, int BufSize)
{
    //Build a ring buffer for UART RX
    ;
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
        if (idx > TERM_STRING_BUF_SIZE)
        {
            idx = 0;
        }
        return (val);
    }
}

int term_history_push(stTermBuf *termbuf)
{

    termbuf->his_pull_idx = termbuf->his_push_idx;

    strcpy(termbuf->history[termbuf->his_push_idx], termbuf->string);

    (termbuf->his_push_idx)++;
    if (termbuf->his_push_idx >= TERM_HISTORY_DEPTH)
    {
        termbuf->his_push_idx = 0;
    }

    return 0;
}

int term_history_pull(stTermBuf *termbuf, int depth)
{
    //Move pull_index
    if (depth > 0)
    {
        if (termbuf->his_pull_idx <= 0)
        {
            termbuf->his_pull_idx = TERM_HISTORY_DEPTH;
        }
        termbuf->his_pull_idx = (termbuf->his_pull_idx) - depth;
    }
    else
    {
        //Already last history, do nothing.
        if (termbuf->his_pull_idx == termbuf->his_push_idx)
        {
            return 0;
        }

        termbuf->his_pull_idx = (termbuf->his_pull_idx) - depth;
        if (termbuf->his_pull_idx >= TERM_HISTORY_DEPTH)
        {
            termbuf->his_pull_idx = 0;
        }

    }

    printf("\npull[%d]push[%d]\n", termbuf->his_pull_idx, termbuf->his_push_idx);
    printf("[0]%s\n", termbuf->history[0]);
    printf("[1]%s\n", termbuf->history[1]);
    printf("[2]%s\n", termbuf->history[2]);
    printf("[3]%s\n", termbuf->history[3]);
    printf("[4]%s\n", termbuf->history[4]);
    printf("[5]%s\n", termbuf->history[5]);
    printf("[6]%s\n", termbuf->history[6]);
    printf("[7]%s\n", termbuf->history[7]);

    //Show history from pull_index
    if (termbuf->history[termbuf->his_pull_idx][0] != 0)
    {
        strcpy(termbuf->string, termbuf->history[termbuf->his_pull_idx]);
        termbuf->index = strlen(termbuf->string);
    }

    return 0;
}

int term_esc_handle(char c, stTermBuf *termbuf)
{
    static char escbuf[8] =
    { 0 };
    static int escidx = 0;

    if (c == '\e')
    {
        termbuf->esc_flag = 2;
    }

    escbuf[escidx++] = c;

    if (strcmp(escbuf, "\e[A") == 0)    //Up Arrow
    {
        term_history_pull(termbuf, 1);
        printf("%s\r>%s", TERM_ERASE_LINE_START, termbuf->string);
    }
    else if (strcmp(escbuf, "\e[B") == 0) //Down Arrow
    {
        term_history_pull(termbuf, -1);
        printf("%s\r>%s", TERM_ERASE_LINE_START, termbuf->string);
    }
    else if (strcmp(escbuf, "\e[C") == 0)   //Right arrow
    {
        if (termbuf->string[termbuf->index] != 0)
        {
            termbuf->index++;
            printf("%s", TERM_CURSOR_RIGHT);
        }
    }
    else if (strcmp(escbuf, "\e[D") == 0)   //Left arrow
    {
        if (termbuf->index > 0)
        {
            termbuf->index--;
            printf("%s", TERM_CURSOR_LEFT);
        }

    }

    if (escidx > 2)
    {
        memset(escbuf, 0, 8);
        escidx = 0;
    }
    return 0;

}

int Term_IO_init()
{
#if TERM_IO_OS == TERM_OS_OSX
    return osx_initTerm();
#endif

#if TERM_IO_OS == TERM_OS_STM32
    return stm32_initTerm();
#endif
}

char Term_IO_getc(void)
{
#if TERM_IO_OS == TERM_OS_OSX
    return osx_getc(stdin);
#endif

#if TERM_IO_OS == TERM_OS_STM32
    return stm32_getc();
#endif
}

int Term_IO_gets(char *dest_str, stTermBuf *termbuf)
{
    //Initialize terminal IO
    static int initflag = 1;
    if (initflag)
    {
        Term_IO_init();
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
        case '\xff': //EOF
        {
            break;
        }
        case '\x7f': //Delete
        case '\b': //Backspace
        {
            if (termbuf->index > 0)
            {
                termbuf->string[termbuf->index--] = 0;
            }
            printf("%s%s", TERM_CURSOR_LEFT, TERM_ERASE_LINE_END);
            break;
        }
        case '\e': //ESC
        {
            term_esc_handle(c, termbuf);
            break;
        }
        case CLI_LINE_END_CHAR: //End of a line
        {
            //Push to history without \'n'
            if (termbuf->index > 0)
            {
                term_history_push(termbuf);
            }

            //Copy out string with '\n'
            termbuf->string[termbuf->index++] = c;
            strcpy(dest_str, termbuf->string);
            printf("\n");

            //Clear buffer
            int ret = termbuf->index;
            memset(termbuf->string, 0, TERM_STRING_BUF_SIZE);
            termbuf->index = 0;

            return ret; //return copy data count
        }
        default:
        {
            if (termbuf->esc_flag > 0)
            {
                term_esc_handle(c, termbuf);
                termbuf->esc_flag--;
            }
            else
            {
                //Buffer 1 byte
                termbuf->string[termbuf->index++] = c;
                //Loop Back function
                printf("%c", c);
            }
            break;
        }
        }
    } while (c != '\xff');

    return 0;
}

