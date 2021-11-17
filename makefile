CC=cc
CFLAGS=-c -Wall -Wextra -std=c11 -ggdb -O0 -I./inc -D_DEFAULT_SOURCE
SOURCES=$(wildcard src/*.c)
OBJECTS=$(SOURCES:.c=.o)
EXE=diffs

$(EXE): $(SOURCES) $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(EXE)
