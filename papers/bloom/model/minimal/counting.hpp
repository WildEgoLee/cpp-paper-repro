// Counting Bloom Filter.
// Each bit is replaced by a small counter so deletion is possible.
// Still no false negatives; false positives possible.

#pragma once
#include <vector>
#include <cstddef>

class CountingBloom {
public:
  CountingBloom(size_t m, size_t k) : counters_(m, 0), k_(k) {}

  void insert(int key) {
    for (size_t i = 0; i < k_; ++i)
      ++counters_[hash(key, i)];
  }

  void erase(int key) {
    for (size_t i = 0; i < k_; ++i) {
      auto& c = counters_[hash(key, i)];
      if (c > 0) --c;
    }
  }

  bool may_contain(int key) const {
    for (size_t i = 0; i < k_; ++i)
      if (counters_[hash(key, i)] == 0) return false;
    return true;
  }

  size_t bit_count() const { return counters_.size(); }

private:
  size_t hash(int key, size_t i) const {
    unsigned h = static_cast<unsigned>(key) * 2654435761u;
    h ^= static_cast<unsigned>(i) * 0x9e3779b9u;
    return h % counters_.size();
  }

  std::vector<unsigned char> counters_;
  size_t k_;
};
