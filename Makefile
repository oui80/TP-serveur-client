CFLAGS= -g -Wall -Werror -DDEBUG_ARBRE
CC=gcc
all : client server

client: client.c client.h
	$(CC) $(CFLAGS) $^ -o $@

server: server.c server.h
	$(CC) $(CFLAGS) $^ -o $@

