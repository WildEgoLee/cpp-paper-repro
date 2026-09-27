CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pthread
LDFLAGS ?= -pthread

.PHONY: all clean test

all: mechanism_probe

mechanism_probe: mechanism_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

test: mechanism_probe
	./mechanism_probe --self-test

clean:
	rm -f mechanism_probe
