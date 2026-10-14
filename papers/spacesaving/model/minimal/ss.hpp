// Minimal Space-Saving interfaces.
// Fixed k counters; replace the minimum on new item.

#pragma once
#include <optional>
#include <cstddef>

class SpaceSaving {
public:
  explicit SpaceSaving(size_t k);

  void add(int key);
  // Returns the estimated count if tracked, else the current minimum.
  int estimate(int key) const;
  // True if the key is currently among the tracked counters.
  bool is_tracked(int key) const;

private:
  // counters_[k] = (key, count)
};
