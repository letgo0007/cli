/*
 * terminal_io.h
 *
 *  Created on: Apr 3, 2018
 *      Author: nickyang
 */

#ifndef TERM_IO_H_
#define TERM_IO_H_

#define TERM_STRING_BUF_SIZE        128     //!< Maximum terminal command string length.
#define TERM_TOKEN_AMOUNT           16      //!< Maximum tokens in a command.
#define TERM_HISTORY_DEPTH          8       //!< History depth.
#define TERM_LOOP_BACK_EN           1       //!< Enable loop back function for terminal.

typedef struct stTermBuf
{
    char string[TERM_STRING_BUF_SIZE];      //!< Command string buffer
    int index;                              //!< Command string w/r index
    int esc_flag;                           //!< Flags to handle ANSI ESC sequence (up/down/left/right)
    _Bool loopback_enable;                  //!< Enable Loopback function for terminal
#if TERM_HISTORY_DEPTH
    char history[TERM_HISTORY_DEPTH][TERM_STRING_BUF_SIZE]; //!< Buffer to store history command
    int his_push_idx;                       //!< History push index
    int his_pull_idx;                       //!< History pull index
#endif
} stTermHandle;

extern stTermHandle gTermHandle;

int Term_IO_init(void);
char Term_IO_getc(void);
int Term_IO_gets(char *string, stTermHandle *TermHandle);

#endif /* TERM_IO_H_ */
