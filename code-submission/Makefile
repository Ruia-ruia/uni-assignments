CC=gcc
CFLAGS=-pg -O0 -Werror=vla -std=gnu11 -g -fsanitize=address -pthread -lrt -lm
PERFLAGS=-O0 -march=native -Werror=vla -std=gnu11 -pthread -lrt -lm
TESTFLAGS=-O0 -Werror=vla -std=gnu11 -g -fprofile-arcs -ftest-coverage -fsanitize=address -pthread -lrt -lm
NAME=btreestore
OBJECT=lib$(NAME).o
LIBRARY=lib$(NAME).a

HEADERS=btreestore.h
SRC=btreestore.c
SRCA=btreestore.c encryption.c print.c delete.c insert.c linkedlist.c node.c
OBJECTS=output.o btreestore.o encryption.o print.o delete.o insert.o linkedlist.o node.o

correctness: $(SRCA)
	$(CC) $(CFLAGS) -c $(SRCA)
	ld -r -o $(OBJECTS)
	ar rcs $(LIBRARY) output.o
	rm -f *.o 

performance: $(SRCA)
	$(CC) $(PERFLAGS) -c $(SRCA)
	ld -r -o $(OBJECTS)
	ar rcs $(LIBRARY) output.o

tests: $(SRCA)
	$(CC) $(CFLAGS) -c $(SRCA)
	ld -r -o $(OBJECTS)
	ar rcs $(LIBRARY) output.o
	rm -f *.o

testing: $(SRCA)
	$(CC) $(CFLAGS) $(SRCA) tests.c -o testing

clean_test:
	rm -f testing

clean:
	rm -f testing
	rm -f *.a
	rm -f *.o
