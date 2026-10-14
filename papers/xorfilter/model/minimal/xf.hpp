// Minimal Xor Filter interfaces.
// Static set; three hashes XOR to fingerprint.

#pragma once
#include <vector>

class XorFilter {
public:
  // Build from a known set of keys.
  explicit XorFilter(const std::vector<int>& keys);

  bool contains(int key) const;

private:
  // table_ stores values such that XOR of three slots equals fingerprint
};
