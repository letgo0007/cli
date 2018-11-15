/******************************************************************************
 * @file    cli.h
 * @brief   Command Line Interface (CLI) for MCU.
 *
 * @author  Nick Yang
 * @date    2018/11/01
 * @version V1.0
 *****************************************************************************/

#ifndef CLI_H_
#define CLI_H_

/*! Includes ----------------------------------------------------------------*/
/*! Defines -----------------------------------------------------------------*/

/*!@defgroup ANSI flow control Escape sequence define.
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

/*!@defgroup CLI return code defines
 *
 */
#define CLI_OK                      0       //!< General success.
#define CLI_FAIL                    -1      //!< General fail.
#define CLI_ERROR_COMMAND           -2      //!< Command is unknown or wrong syntax.
#define CLI_ERROR_PARAM             -3      //!< Parameter is wrong or missing
#define CLI_ERROR_UNKNOWN_CHAR      -4      //!< Get character

/*!@defgroup CLI operation defines
 *
 */
#define CLI_PROMPT_CHAR             ">"     //!< Prompt character shows at the head of line
#define CLI_PROMPT_LEN              strlen(CLI_PROMPT_CHAR)
#define CLI_STR_BUF_SIZE            256     //!< Maximum command length
#define CLI_ARGC_MAX                32      //!< Maximum arguments in a command
#define CLI_COMMAND_SIZE            32      //!< Number of commands in the list
#define CLI_VERSION                 "1.0.0"

/*!@defgroup CLI history function defines
 *
 */
#define HISTORY_ENABLE              1       //!< Enable history function
#define HISTORY_DEPTH               32      //!< Maximum number of command saved in history
#define HISTORY_MEM_SIZE            256     //!< Maximum RAM usage for history

/*!@typedef CliCommand_TypeDef
 *          Structure for a CLI command.
 */
typedef struct
{
    const char *Name;                       //!< Command Name
    const char *Prompt;                     //!< Prompt text
    int (*Func)(int argc, char **argv);     //!< Function call
} CliCommand_TypeDef;

/*!@typedef CliOption_TypeDef
 *          Structure for a CLI command options. It's a implement of the "getopt" & "getopt_long" function.
 * @example see "builtin_test" function
 */
typedef struct
{
    const char ShortName;                   //!< Short name work with "-", e.g. 'h'
    const char *LongName;                   //!< Long name work with "--", e.g. "help"
    const int ReturnVal;         //!< Return value if this option is detected. Use short name would be the simplest way.
} CliOption_TypeDef;

/*! Variables ---------------------------------------------------------------*/
/*! Functions ---------------------------------------------------------------*/

int Cli_Register(const char *name, const char *prompt, int (*func)(int, char **));
int Cli_Unregister(const char *name);
int Cli_RunByArgs(int argcount, char **argbuf);
int Cli_RunByString(char *cmd);
int Cli_Init(void);
int Cli_Run(void);
int Cli_Task(void);

#endif /* CLI_H_ */
