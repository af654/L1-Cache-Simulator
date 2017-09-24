PROGRAMS = c-sim

CC = gcc
CFLAGS = -Wall -lm -g

%: %.c %.h
	$(CC) $(CFLAGS) -o $@ $<

.PHONY: all clean

all: $(PROGRAMS)

clean:
	@rm -f *.o $(PROGRAMS)
