CC=cc
CFLAGS=-c -Wall -Wextra -std=c11 -ggdb -O0
SOURCES=$(wildcard *.c)
OBJECTS=$(SOURCES:.c=.o)
EXE=trav

$(EXE): $(SOURCES) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXE)
