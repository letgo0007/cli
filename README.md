# cli
A Command Line Interface for MCU.
The basic syntax is
```
[ Command ][                 Arguments                ]
test        --timeout 100 --repeat --file /path/to/file   //Long format
test        -t 100 -r -f /path/to/file                    //Short format
```
The steps for processing the command 
1. Serach a dictory of `Commands` to find the function to run, pass `Arguments` to the function.
2. Parse the `Arguments` in the syntax of `-` & `--`.
