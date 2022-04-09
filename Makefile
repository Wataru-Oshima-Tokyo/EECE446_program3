EXE = peer
CFLAGS = -Wall
CXXFLAGS = -Wall
LDLIBS =
CC = gcc
CXX = g++

.PHONY: all
all: $(EXE)

# Implicit rules defined by Make, but you can redefine if needed
#
peer: peer.c
	$(CC) $(CFLAGS) peer.c $(LDLIBS) -o peer
#
# OR
#
#h1-counter: h1-counter.cc
#       $(CXX) $(CXXFLAGS) h1-counter.cc $(LDLIBS) -o h1-counter

.PHONY: clean
clean:
	rm -f $(EXE)

