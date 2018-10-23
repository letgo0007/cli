/*
 * cli_simple.h
 *
 *  Created on: Oct 23, 2018
 *      Author: nickyang
 */

#ifndef CLI_SIMPLE_H_
#define CLI_SIMPLE_H_

/*! ANSI escape code
 *  Terminal flow control with ANSI escape code.
 *  Refer to <https://en.wikipedia.org/wiki/ANSI_escape_code>
 */

#define ANSI_CURSOR_UP              "\e[A"
#define ANSI_CURSOR_DOWN            "\e[B"
#define ANSI_CURSOR_RIGHT           "\e[C"
#define TERM_CURSOR_LEFT            "\e[D"
#define TERM_CURSOR_NEXT_LINE       "\e[E"
#define TERM_CURSOR_PREVIOUS_LINE   "\e[F"
#define TERM_CURSOR_SAVE_POSITION   "\e[s"
#define TERM_CURSOR_RESTORE_POSITION "\e[u"

#define TERM_ERASE_DISPLAY          "\e[J"
#define TERM_ERASE_LINE_START       "\e[1K"
#define TERM_ERASE_LINE_END         "\e[K"
#define TERM_ERASE_LINE             "\e[2K"

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

#define TERM_PROMPT_CHAR            ">"
#define TERM_PROMPT_LEN             strlen(TERM_PROMPT_CHAR)


typedef struct
{
    char *Name;                         //!< Command Name
    int (*Func)(int argc, char **argv);   //!< Function call back
    char *HelpText;                     //!< Text to describe the function.
} TermCommand_TypeDef;

int Simple_IO_gets(char *dest_str);

#endif /* CLI_SIMPLE_H_ */
