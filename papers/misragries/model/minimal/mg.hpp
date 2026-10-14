// Minimal Misra-Gries interfaces.
// Fixed counters; decrement all on new item when full.

#pragma once
#include <cstddef>

class MisraGries {
public:
  explicit MisraGries(size_t k);

  void add(int key);
  // True if the key is currently among the tracked counters.
  bool is_tracked(int key) const;

private:
  // counters_[k-1]
};
