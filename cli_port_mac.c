/******************************************************************************
 * @file    cli_porting_mac.c
 * @brief   A simple Command Line Interface (CLI) for MCU.
 *          This file contains the API that needs to port for your system.
 *          This file give a example for MacOS.
 *
 * @author  Nick Yang
 * @date    2018/11/01
 * @version V1.0
 *****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_MAC
#include <fcntl.h>
#include <sys/timeb.h>
#include <termios.h>
#include <unistd.h>
#endif
#endif

#include "cli.h"

void cli_sleep(float s)
{
    usleep(s * 1000000);
}

/*!@brief Get system tick in ms.
 *
 * @return
 */
unsigned int cli_gettick(void)
{
    struct timeb tm;
    ftime(&tm);
    return (unsigned int) (tm.time * 1000 + tm.millitm);
}

void *cli_malloc(size_t size)
{

    void *ptr = NULL;
    while (ptr == NULL)
    {
        ptr = malloc(size);
    }
    memset(ptr, 0, size);
    return ptr;
}

void cli_free(void *ptr)
{
    free(ptr);
}

int cli_port_init()
{
    struct termios new, old;
    int flag;

    // Disable Echo function for STDIN
    tcgetattr(STDIN_FILENO, &old);
    new = old;
    new.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL | ICANON);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);

    // Set STDIN to Non-blocking mode.
    flag = fcntl(STDIN_FILENO, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(STDIN_FILENO, F_SETFL, flag) == -1)
    {
        return (-1);
    }

    setvbuf(stdout, (char *) NULL, _IOLBF, 1024);
    setvbuf(stderr, (char *) NULL, _IONBF, 1);
    setvbuf(stdin, (char *) NULL, _IONBF, 1);

    return 0;
}

void cli_port_deinit()
{
    ;
}

int cli_port_getc(void)
{
    return getchar();
}
