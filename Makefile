#Copyright @2018 Nick Yang

###Compiler
CC=gcc

###C source file
CSOURCE=\
main.c\
cli_simple.c

###C include path
CINCLUDE=-I.

###C flags
CFLAG=-Wall

###Lib search path
LIBPATH=

###Lib flags, make sure libft4222.dylib is in /usr/local/lib
LIBFLAG=

###TARGET
TARGET=cli

all:
	$(CC) $(CSOURCE) $(CINCLUDE) $(CFLAG) -o$(TARGET)$(LIBPATH) $(LIBFLAG)
	
debug: all
	./cli

clean: 
	rm -f $(TARGET)