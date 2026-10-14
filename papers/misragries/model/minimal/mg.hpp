// Minimal Misra-Gries.
// Fixed counters; decrement all on new item when full.

#pragma once
#include <vector>
#include <optional>
#include <cstddef>

class MisraGries {
public:
  explicit MisraGries(size_t k) : counters_(k > 0 ? k - 1 : 0) {}

  void add(int key) {
    for (auto& c : counters_) {
      if (c && c->key == key) { ++c->count; return; }
    }
    // Find empty slot.
    for (auto& c : counters_) {
      if (!c) { c = Slot{key, 1}; return; }
    }
    // Full: decrement all, drop zeros.
    for (auto& c : counters_) {
      if (c) {
        --c->count;
        if (c->count == 0) c.reset();
      }
    }
  }

  bool is_tracked(int key) const {
    for (auto& c : counters_)
      if (c && c->key == key) return true;
    return false;
  }

private:
  struct Slot { int key; int count; };
  std::vector<std::optional<Slot>> counters_;
};
