VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra

all: server

server: server.c http_server.c
	gcc server.c http_server.c $(CFLAGS) -o server
