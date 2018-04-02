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

#define TER_GETS_NON_BLOCK          1       //!< Use Non-blocking mode input.
#define TER_STRING_BUF_SIZE         128     //!< Terminal input string buffer size.
#define TER_HISTORY_DEPTH           4
#define TER_LOOP_BACK_EN            1       //!< Enable loop back function for terminal.

typedef struct stTermHistory
{
    char strbuf[TER_HISTORY_DEPTH][TER_STRING_BUF_SIZE];
    int depth;
    int index;
} stTermHistory;

extern stCliCommand MainCmd_V1[];
extern stTermHistory TermHistory;

int Terminal_run(stCliCommand cmdlist[]);

#endif /* TERMINAL_H_ */
