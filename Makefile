CFLAGS= -g -Wall -Werror -DDEBUG_ARBRE
CC=gcc
all : client server

client: client.c
	$(CC) $(CFLAGS) $^ -o $@

server: server.c
	$(CC) $(CFLAGS) $^ -o $@

