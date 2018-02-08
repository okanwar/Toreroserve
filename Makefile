CC=gcc
CXX=g++

CXXFLAGS=-Wall -Wextra -g -O1 -std=c++11

CFLAGS=-Wall -Wextra -g -O1 -pthread

TARGETS=toreroserve thread_example

all: $(TARGETS)

toreroserve: toreroserve.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

thread_example: thread_example.c
	gcc $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGETS)
