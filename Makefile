CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude -g

SRCS = src/text_buffer.c src/editoperation.c src/version.c src/server.c src/client.c

all: server client

server: src/text_buffer.c src/editoperation.c src/version.c src/server.c
	$(CC) $(CFLAGS) src/text_buffer.c src/editoperation.c src/version.c src/server.c -o server

client: src/text_buffer.c src/editoperation.c src/version.c src/client.c
	$(CC) $(CFLAGS) src/text_buffer.c src/editoperation.c src/version.c src/client.c -o client

clean:
	rm -f server client