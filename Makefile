VFLAGS = --track-origins=yes --leak-check=full --show-leak-kinds=all
CFLAGS = -std=gnu11 -g -Wall -Wextra -lm

all: server

server: server.c http_server.c http_status.c utils.c
	gcc server.c http_server.c http_status.c utils.c $(CFLAGS) -o server && ./server

valgrind: server.c http_server.c
	gcc server.c http_server.c $(CFLAGS) -o server && valgrind --leak-check=full --show-leak-kinds=all ./server
