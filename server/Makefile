CFLAGS = -c -Wall
LDFLAGS = -lwebsockets
CC = gcc

all: server

server: server.o
	${CC} server.o -o server ${LDFLAGS}

server.o: server.c
	${CC} ${CFLAGS} server.c

run: server
	clear
	./server

clean:
	rm -f *.o *~ server