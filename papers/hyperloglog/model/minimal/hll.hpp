// Minimal HyperLogLog interfaces.
// Registers track max leading zeros; estimate via harmonic mean.

#pragma once
#include <cstddef>

class HyperLogLog {
public:
  explicit HyperLogLog(size_t m);

  void add(int key);
  double estimate() const;

private:
  // registers_[i] = max leading zeros seen in bucket i
};
