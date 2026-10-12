CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I../model/minimal

.PHONY: all clean

all: mechanism_probe

mechanism_probe: mechanism_probe.cpp ../model/minimal/lsm.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f mechanism_probe
