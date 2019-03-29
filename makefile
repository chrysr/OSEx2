CC=g++
CFLAGS=-I.
DEPS=Record.h
OBJ=root.o Record.o merger-splitter.o searcher.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: root searcher merger-splitter

searcher: searcher.o Record.o
	$(CC) -o $@ $^ $(CFLAGS)

root: root.o Record.o
	$(CC) -o $@ $^ $(CFLAGS)

merger-splitter: merger-splitter.o Record.o
	$(CC) -o $@ $^ $(CFLAGS)


.Phony: clean
clean:
	-rm $(OBJ)
	-rm root
	-rm merger-splitter
	-rm searcher
