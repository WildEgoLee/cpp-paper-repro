// Minimal Count Sketch interfaces.
// Sign hash per row; estimate is the median across rows.

#pragma once
#include <cstddef>

class CountSketch {
public:
  CountSketch(size_t d, size_t w);

  void add(int key);
  int estimate(int key) const;   // median of signed counters

private:
  // counters_[d][w]
  // g_[d] sign hash
};
