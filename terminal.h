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
#define TER_STRING_BUF_SIZE         256     //!< Terminal input string buffer size.
#define TER_LOOP_BACK_EN            1       //!< Enable loop back function for terminal.

extern stCliCommand MainCmd_V1[];

int Terminal_run(stCliCommand cmdlist[]);

#endif /* TERMINAL_H_ */
