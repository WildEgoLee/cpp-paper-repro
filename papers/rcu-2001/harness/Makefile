CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread -I../model/minimal

.PHONY: all clean

all: mechanism_probe

mechanism_probe: mechanism_probe.cpp ../model/minimal/rcu.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f mechanism_probe
