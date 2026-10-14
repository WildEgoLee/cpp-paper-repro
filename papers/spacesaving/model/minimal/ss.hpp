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
    size_t idx = 0;
    bool found_empty = false;
    int min_count = 0;
    for (size_t i = 0; i < counters_.size(); ++i) {
      if (!counters_[i]) { idx = i; found_empty = true; break; }
      if (i == 0 || counters_[i]->count < min_count) {
        min_count = counters_[i]->count;
        idx = i;
      }
    }
    int base = found_empty ? 0 : min_count;
    counters_[idx] = Slot{key, base + 1};
  }

  int estimate(int key) const {
    for (auto& c : counters_)
      if (c && c->key == key) return c->count;
    int m = 0;
    bool any = false;
    for (auto& c : counters_)
      if (c && (!any || c->count < m)) { m = c->count; any = true; }
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
