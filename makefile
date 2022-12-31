CC = gcc
CCFLAGS = -fPIC -Wall -std=c11 -Iinclude

all: list parser shared

list:
	$(CC) $(CCFLAGS) -c src/LinkedListAPI.c -o bin/LinkedListAPI.o
		ar rc bin/libllist.a bin/LinkedListAPI.o

parser:
	$(CC) $(CCFLAGS) -c src/CalendarParser.c -o bin/CalendarParser.o
		ar rc bin/libparser.a bin/CalendarParser.o

shared:
	$(CC) -shared -o bin/main.so bin/LinkedListAPI.o bin/CalendarParser.o

clean:
	rm bin/*.o bin/*.a bin/*.so
