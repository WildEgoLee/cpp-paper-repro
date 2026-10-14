CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I../model/minimal

.PHONY: all clean

all: mechanism_probe stall_probe

mechanism_probe: mechanism_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

stall_probe: stall_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f mechanism_probe stall_probe
