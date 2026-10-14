CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I../model/minimal

.PHONY: all clean

all: mechanism_probe fpr_probe counting_probe

mechanism_probe: mechanism_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

fpr_probe: fpr_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

counting_probe: counting_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f mechanism_probe fpr_probe counting_probe
