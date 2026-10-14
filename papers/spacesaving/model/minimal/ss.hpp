// Minimal Space-Saving.
// Fixed k counters; replace the minimum-count item on new arrival.

#pragma once
#include <vector>
#include <optional>
#include <algorithm>
#include <cstddef>

class SpaceSaving {
public:
  explicit SpaceSaving(size_t k) : counters_(k) {}

  void add(int key) {
    for (auto& c : counters_) {
      if (c && c->key == key) { ++c->count; return; }
    }
    // Find empty or minimum.
    auto* target = &counters_[0];
    for (auto& c : counters_) {
      if (!c) { target = &c; break; }
      if (c->count < target->count) target = &c;
    }
    int base = target-> ? target->count : 0;
    *target = Slot{key, base + 1};
  }

  int estimate(int key) const {
    for (auto& c : counters_)
      if (c && c->key == key) return c->count;
    int m = 0;
    for (auto& c : counters_)
      if (c && (m == 0 || c->count < m)) m = c->count;
    return m;
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
