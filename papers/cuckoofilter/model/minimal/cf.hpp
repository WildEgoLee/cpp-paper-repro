// Minimal Cuckoo Filter interfaces.
// Fingerprints in buckets; second bucket derived by XOR.

#pragma once
#include <cstddef>

class CuckooFilter {
public:
  explicit CuckooFilter(size_t buckets);

  bool insert(int key);          // false if kick chain too long
  bool contains(int key) const;
  bool remove(int key);          // true if fingerprint was found

private:
  // buckets_ store fingerprints
};
