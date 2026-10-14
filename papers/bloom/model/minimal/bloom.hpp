// Minimal Bloom Filter interfaces.
// Bit array + k hash functions.
// No false negatives; false positives possible.

#pragma once
#include <vector>
#include <cstddef>

class BloomFilter {
public:
  explicit BloomFilter(size_t m, size_t k);

  void insert(int key);
  bool may_contain(int key) const;   // false → definitely absent

  size_t bit_count() const;

private:
  std::vector<bool> bits_;
  size_t k_;
};
