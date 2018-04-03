/******************************************************************************
 * @file    terminal.h
 *          An example of building a command list for CLI.
 *
 * @author  Nick Yang
 * @date    2018/03/15
 * @version V0.1
 *****************************************************************************/
#ifndef TERMINAL_H_
#define TERMINAL_H_

#include "cli.h"
#include "term_io.h"

/*! ANSI escape code
 *  Terminal flow control with ANSI escape code.
 *  Refer to <https://en.wikipedia.org/wiki/ANSI_escape_code>
 */

#define TERM_CURSOR_UP              "\e[A"
#define TERM_CURSOR_DOWN            "\e[B"
#define TERM_CURSOR_RIGHT           "\e[C"
#define TERM_CURSOR_LEFT            "\e[D"
#define TERM_CURSOR_NEXT_LINE       "\e[E"
#define TERM_CURSOR_PREVIOUS_LINE   "\e[F"

#define TERM_ERASE_DISPLAY          "\e[J"
#define TERM_ERASE_LINE_START       "\e[1K"
#define TERM_ERASE_LINE_END         "\e[K"

#define TERM_RESET                  "\e[0m"
#define TERM_BOLD                   "\e[1m"
#define TERM_ITALIC                 "\e[3m"
#define TERM_UNDERLINE              "\e[4m"
#define TERM_BLINK                  "\e[5m"

#define TERM_BLACK                  "\e[30m"
#define TERM_RED                    "\e[31m"
#define TERM_GREEN                  "\e[32m"
#define TERM_YELLOW                 "\e[33m"
#define TERM_BLUE                   "\e[34m"

extern stCliCommand MainCmd_V1[];

int Terminal_run(stCliCommand cmdlist[]);

#endif /* TERMINAL_H_ */
