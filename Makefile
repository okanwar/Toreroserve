TARGETS=toreroserve thread_example

CFLAGS=-Wall -g -O0 -pthread

all: $(TARGETS)

toreroserve: toreroserve.c
	gcc $(CFLAGS) -o $@ $^

thread_example: thread_example.c
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
