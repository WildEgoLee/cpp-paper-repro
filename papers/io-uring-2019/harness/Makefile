CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread -I../model/minimal

.PHONY: all clean

all: mechanism_probe real_uring_probe real_uring_rw_probe real_uring_batch_probe

mechanism_probe: mechanism_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

real_uring_probe: real_uring_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

real_uring_rw_probe: real_uring_rw_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

real_uring_batch_probe: real_uring_batch_probe.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

clean:
	rm -f mechanism_probe real_uring_probe real_uring_rw_probe real_uring_batch_probe
