// Minimal B+ Tree interfaces.
// Leaves hold data and are linked; internals are separators only.

#pragma once
#include <vector>
#include <optional>

class BPTree {
public:
  void insert(int key, int value);
  std::optional<int> lookup(int key) const;
  // Range scan: all values with keys in [lo, hi].
  std::vector<int> range(int lo, int hi) const;

private:
  // Toy: single leaf chain for the first version.
  struct Leaf {
    std::vector<std::pair<int, int>> entries;
    Leaf* next = nullptr;
  };
  Leaf* head_ = nullptr;
};
