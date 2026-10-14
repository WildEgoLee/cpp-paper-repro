// Minimal HyperLogLog.
// Registers track max leading zeros; estimate via harmonic mean.

#pragma once
#include <vector>
#include <cmath>
#include <cstdint>
#include <algorithm>

class HyperLogLog {
public:
  explicit HyperLogLog(size_t m) : regs_(m, 0) {}

  void add(int key) {
    unsigned h = hash(key);
    size_t idx = h % regs_.size();
    int zeros = leading_zeros(h);
    regs_[idx] = std::max(regs_[idx], zeros);
  }

  double estimate() const {
    double sum = 0;
    for (int r : regs_) sum += std::pow(2.0, -r);
    double alpha = 0.7213 / (1.0 + 1.079 / regs_.size());
    return alpha * regs_.size() * regs_.size() / sum;
  }

private:
  unsigned hash(int key) const {
    return static_cast<unsigned>(key) * 2654435761u;
  }
  int leading_zeros(unsigned h) const {
    int z = 0;
    while ((h & 1) == 0 && z < 16) { ++z; h >>= 1; }
    return z;
  }

  std::vector<int> regs_;
};
