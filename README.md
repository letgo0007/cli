cli
===

A CLI(Command Line Interface) example build in pure C. Designed for MCU, support block / non-block mode input.

File tree
=========

-	`<cli.c>` `<cli.h>`
	-	Basic CLI function library.
-	`<terminal.c>` `<terminal.h>`
	-	An example of terminal application with some basic command.
-	`<term_io.c>` `<term_io.h>`
	-	An example of terminal IO stream handle with history, delete, insert function support.

Complie
=======

```
make all | clean | debug
```

How it works
============

STEP1: Build a command list.
----------------------------

Register all needed CLI function and link the function call back.

```
>help
help    Show the command list
test    Run a argument parse example.
time    Get current time stamp
quit    Quit the process
version Show Command version
echo    Echo back command
history Show command history
```

STEP2: Build option list for each command if needed.
----------------------------------------------------

Here is a simple implement to handle options with prefix with `-` & `--`.

```
>test -h
Basic Options:
	-h   --help      :Show help hints
	-i   --int       :Get a Integer value
	-s   --string    :Get a String value
	-b   --bool      :Get a Boolean value
```

STEP3: Check input characters
-----------------------------

-	In `block mode`, the process will block until user press `ENTER`. This is usually the default method when you are using `stdio` functions in OS environment. Flow control like `delete` `back space` `left arrow` & etc is handled by OS and your will get a pure string ended with `\n`.
-	In `non-blocking mode`, it works more efficiently on a general MCU application. Assume your are buffering UART received characters in a ring buffer, and the CLI will check this buffer area on every loop until getting a certain character like `\n`. Here also provide a example of handling flow control of `delete` `arrow` and history callback since usually you don't have it in a MCU library.

STEP3: Parse input string
-------------------------

1.	Assume the input string is`"test -i 123\n"`
2.	Break down the string to tokens by defined separator (e.g. space)`"test"` `"-i"` `"123"`
3.	Search the 1st token in the `stCliCommand` list to determine which fucntion to call, here your are calling `test` with the reset tokens`int Command_test(2, "-i" "123")`
4.	Parse the tokens with given `stCliOption` list, here `-i` is defined like this:`{ OPT_INT, 'i', "int", "Get a Integer value", (void*) &Tempdata.IntData },` So `"123"` is defined as integer and it will be convert to a int value and pass to defined pointer.

```
>test -i 123
Result: Int[123] String[] Bool[0]
```
