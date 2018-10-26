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
#if TARGET_OS_MAC
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#endif
#endif

#ifdef STM32L4
#include "stm32l4xx.h"
#include "cmsis_os.h"
#endif

#include "cli.h"

void cli_sleep(int ms)
{
#if  defined(TARGET_OS_MAC)
    usleep(ms * 1000);
#elif defined(osCMSIS)
    osDelay(ms);
#elif defined(__STM32L4_CMSIS_VERSION)
    HAL_Delay(ms)
#endif
}

void* cli_malloc(size_t size)
{

    void *ptr = NULL;
    while (ptr == NULL)
    {
#if  defined(osCMSIS)
        ptr = pvPortMalloc(size);
#else
        ptr = malloc(size);
#endif
    }
    memset(ptr, 0, size);
    return ptr;
}

void cli_free(void* ptr)
{
#if  defined(osCMSIS)
    vPortFree(ptr);
#else
    free(ptr);
#endif
}

int cli_port_init()
{
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
#endif

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
