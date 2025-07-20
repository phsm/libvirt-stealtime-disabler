CC=gcc
CFLAGS=-Wall -Werror -shared -fPIC 

default: injectflag.so

%.so: %.c
	$(CC) $(CFLAGS) -o $@ $<
