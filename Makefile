#Copyright @2018 Nick Yang

###Compiler
CC = gcc

###C source file
CSOURCE= \
main.c\
cli.c\
terminal.c

###C include path
CINCLUDE = -I.

###C flags
CFLAG = -Wall

###Lib search path
LIBPATH =

###Lib flags, make sure libft4222.dylib is in /usr/local/lib
LIBFLAG =

###TARGET
TARGET = cli

all:
	$(CC) $(CSOURCE) $(CINCLUDE) $(CFLAG) $(LIBPATH) $(LIBFLAG) -o$(TARGET)
	
debug: all
	./cli

clean: 
	rm -f $(TARGET)