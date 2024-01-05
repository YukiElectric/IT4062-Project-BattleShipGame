CFLAGS = -std=c++11 -Wall -c
LDFLAGS = -lsqlite3
CC = g++

all: battleship
battleship.o: battleship.cpp
	${CC} ${CFLAGS} battleship.cpp ${LDFLAGS}

battleship: battleship.o
	${CC} battleship.o -o battleship ${LDFLAGS}
	clear

run: battleship
	clear
	./battleship

clean:
	rm -f *.o *~ battleship