CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

.PHONY: all clean

all: mechanism_probe

mechanism_probe: mechanism_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f mechanism_probe
