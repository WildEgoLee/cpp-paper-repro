// Minimal MinHash interfaces.
// Signature of k minimum hashes; similarity is the agreement fraction.

#pragma once
#include <cstddef>
#include <vector>

class MinHash {
public:
  explicit MinHash(size_t k);

  // Build a signature for a set of integers.
  std::vector<unsigned> signature(const std::vector<int>& set) const;

  // Fraction of positions where the two signatures agree.
  double similarity(const std::vector<unsigned>& a,
                    const std::vector<unsigned>& b) const;

private:
  size_t k_;
};
