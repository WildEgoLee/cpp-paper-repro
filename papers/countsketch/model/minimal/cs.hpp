// Minimal Count Sketch.
// Sign hash per row; estimate is the median across rows.

#pragma once
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cstdlib>

class CountSketch {
public:
  CountSketch(size_t d, size_t w)
      : d_(d), w_(w), counters_(d, std::vector<int>(w, 0)) {}

  void add(int key) {
    for (size_t r = 0; r < d_; ++r)
      counters_[r][hash(r, key)] += sign(r, key);
  }

  int estimate(int key) const {
    std::vector<int> vals;
    vals.reserve(d_);
    for (size_t r = 0; r < d_; ++r)
      vals.push_back(sign(r, key) * counters_[r][hash(r, key)]);
    std::sort(vals.begin(), vals.end());
    return vals[vals.size() / 2];
  }

private:
  size_t hash(size_t row, int key) const {
    unsigned h = static_cast<unsigned>(key) * (2654435761u + row * 97);
    return h % w_;
  }
  int sign(size_t row, int key) const {
    unsigned h = static_cast<unsigned>(key) * (0x9E3779B9u + row * 13);
    return (h & 1) ? 1 : -1;
  }

  size_t d_, w_;
  std::vector<std::vector<int>> counters_;
};
