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
#define ANSI_CURSOR_LEFT            "\e[D"
#define ANSI_CURSOR_NEXT_LINE       "\e[E"
#define ANSI_CURSOR_PREVIOUS_LINE   "\e[F"
#define ANSI_CURSOR_SAVE_POSITION   "\e[s"
#define ANSI_CURSOR_RESTORE_POSITION "\e[u"

#define ANSI_ERASE_DISPLAY          "\e[J"
#define ANSI_ERASE_LINE_START       "\e[1K"
#define ANSI_ERASE_LINE_END         "\e[K"
#define ANSI_ERASE_LINE             "\e[2K"

#define ANSI_RESET                  "\e[0m"
#define ANSI_BOLD                   "\e[1m"
#define ANSI_ITALIC                 "\e[3m"
#define ANSI_UNDERLINE              "\e[4m"
#define ANSI_BLINK                  "\e[5m"

#define ANSI_BLACK                  "\e[30m"
#define ANSI_RED                    "\e[31m"
#define ANSI_GREEN                  "\e[32m"
#define ANSI_YELLOW                 "\e[33m"
#define ANSI_BLUE                   "\e[34m"

#define TERM_PROMPT_CHAR            ">"
#define TERM_PROMPT_LEN             strlen(TERM_PROMPT_CHAR)

typedef struct
{
    const char *Name;                   //!< Command Name
    const char *Prompt;                 //!< Prompt text
    int (*Func)(int argc, char **argv); //!< Function call
} CliCommand_TypeDef;

int Simple_IO_gets(char *dest_str);
int Cli_Register(const char *name, const char *prompt, int (*func));
int Cli_Unregister(const char *name);
int Cli_RunArgs(int argcount, char **argbuf);
int Cli_RunString(char *cmd);

#endif /* CLI_SIMPLE_H_ */
