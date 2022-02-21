//        |----------------------------------| 
//        |           Scott Maciver          |
//        |                                  |
//        |                                  |
//        |     A S S I G N M E N T  2       |
//        |----------------------------------|

CC=g++
CFLAGS=-I
CFLAGS+=-Wall
FILES=LogServer.cpp
LIBS=-lpthread

server: $(FILES)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)

clean:
	rm -f *.o server
	
all: server
