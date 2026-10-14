// Minimal Bloom Filter.
// Bit array + k simple hash functions.
// No false negatives; false positives possible.

#pragma once
#include <vector>
#include <cstddef>

class BloomFilter {
public:
  BloomFilter(size_t m, size_t k) : bits_(m, false), k_(k) {}

  void insert(int key) {
    for (size_t i = 0; i < k_; ++i)
      bits_[hash(key, i)] = true;
  }

  bool may_contain(int key) const {
    for (size_t i = 0; i < k_; ++i)
      if (!bits_[hash(key, i)]) return false;
    return true;
  }

  size_t bit_count() const { return bits_.size(); }

  size_t set_bits() const {
    size_t n = 0;
    for (bool b : bits_) if (b) ++n;
    return n;
  }

private:
  size_t hash(int key, size_t i) const {
    // Two independent-looking hashes mixed with the index.
    unsigned h = static_cast<unsigned>(key) * 2654435761u;
    h ^= static_cast<unsigned>(i) * 0x9e3779b9u;
    return h % bits_.size();
  }

  std::vector<bool> bits_;
  size_t k_;
};
