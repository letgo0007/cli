/*
 * terminal_io.h
 *
 *  Created on: Apr 3, 2018
 *      Author: nickyang
 */

#ifndef TERM_IO_H_
#define TERM_IO_H_

#define TERM_MODE_BLOCK             0
#define TERM_MODE_NONBLOCK          1

#define TERM_OS_OSX                 0
#define TERM_OS_STM32               1

#define TERM_IO_MODE                TERM_MODE_NONBLOCK
#define TERM_IO_OS                  TERM_OS_OSX

#define TERM_STRING_BUF_SIZE        256
#define TERM_HISTORY_DEPTH          8

typedef struct stTermBuf
{
    char string[TERM_STRING_BUF_SIZE];
    int index;
    char history[TERM_HISTORY_DEPTH][TERM_STRING_BUF_SIZE];
    int his_push_idx;
    int his_pull_idx;
    int esc_flag;

} stTermBuf;

extern stTermBuf TermBuf;

int Term_IO_init(void);
int Term_IO_gets(char *string, stTermBuf *termbuf);

#endif /* TERM_IO_H_ */
