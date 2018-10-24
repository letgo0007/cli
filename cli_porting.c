/*
 * cli_porting.c
 *
 *  Created on: Oct 23, 2018
 *      Author: nickyang
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#ifdef TARGET_OS_MAC
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif
#endif

#include "cli.h"

unsigned int CliMemUsage = 0;

void cli_sleep(int ms)
{
#ifdef TARGET_OS_MAC
    usleep(ms * 1000);
#endif
}

void* cli_malloc(size_t size)
{
#ifdef TARGET_OS_MAC
    void *ptr = NULL;
    while (ptr == NULL)
    {
        ptr = malloc(size);
    }
    memset(ptr, 0, size);
    CliMemUsage += size;
    printf("request memory = %d\n",size);
    return ptr;
#endif
}

void cli_free(void* ptr)
{
#ifdef TARGET_OS_MAC
    free(ptr);
#endif
}

int cli_io_init()
{
    printf("Complier Version  = %s\n", __VERSION__);
    printf("Compile Date/Time = %s %s\n", __DATE__, __TIME__);

#ifdef TARGET_OS_MAC
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
    printf("Platform          = %s\n", "MacOs");
    printf("Memory Usage      = %d Byte\n", CliMemUsage);
    printf("STDIN set to non-blocking mode\n%s", CLI_PROMPT_CHAR);
#endif

    return 0;
}

void cli_io_deinit()
{
    ;
}

int cli_io_getc(void)
{
    return getchar();
}
