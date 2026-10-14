// Minimal Count-Min Sketch interfaces.
// d rows, w columns; estimate is the minimum across rows.

#pragma once
#include <cstddef>

class CountMinSketch {
public:
  CountMinSketch(size_t d, size_t w);

  void add(int key);
  int estimate(int key) const;   // >= true frequency

private:
  // counters_[d][w]
};
