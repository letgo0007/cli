#Copyright @2018 Nick Yang

###Compiler
CC=gcc

###C source file
CSOURCE=\
main.c\
cli_port_mac.c \
cli.c

###C include path
CINCLUDE=-I.

###C flags
CFLAG=-Wall -g -O0

###Lib search path
LIBPATH=

###Lib flags, make sure libft4222.dylib is in /usr/local/lib
LIBFLAG=

###TARGET
TARGET=cli

all:
	$(CC) $(CFLAG) $(LIBPATH) $(LIBFLAG) $(CINCLUDE) $(CSOURCE) -o$(TARGET)
	
### Debug with lldb. see @ http://lldb.llvm.org/lldb-gdb.html
debug: all
	lldb cli -o "process launch --tty"

run: all
	./$(TARGET)
	
clean: 
	rm -f $(TARGET)