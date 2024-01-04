CFLAGS = -c -Wall
LDFLAGS = -lwebsockets -lcjson -lssl -lcrypto -lmysqlclient
CC = gcc

all: server

server: server.o
	${CC} server.o -o server ${LDFLAGS}
	clear

server.o: server.c
	${CC} ${CFLAGS} server.c

run: server
	clear
	./server

clean:
	rm -f *.o *~ server
