// Minimal Count-Min Sketch.
// d rows, w columns; estimate is the minimum across rows.
// Never underestimates.

#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>

class CountMinSketch {
public:
  CountMinSketch(size_t d, size_t w)
      : d_(d), w_(w), counters_(d, std::vector<int>(w, 0)) {}

  void add(int key) {
    for (size_t r = 0; r < d_; ++r)
      ++counters_[r][hash(r, key)];
  }

  int estimate(int key) const {
    int m = counters_[0][hash(0, key)];
    for (size_t r = 1; r < d_; ++r)
      m = std::min(m, counters_[r][hash(r, key)]);
    return m;
  }

private:
  size_t hash(size_t row, int key) const {
    unsigned h = static_cast<unsigned>(key) * (2654435761u + row * 97);
    return h % w_;
  }

  size_t d_, w_;
  std::vector<std::vector<int>> counters_;
};
