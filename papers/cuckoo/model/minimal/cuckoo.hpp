// Minimal Cuckoo Hashing interfaces.
// Two tables, two hash functions, kick-out on insert.

#pragma once
#include <optional>

class CuckooHash {
public:
  explicit CuckooHash(size_t capacity);

  bool insert(int key);          // false if rehash needed (toy: just fail)
  bool contains(int key) const;
  size_t kicks() const;          // how many kick-outs happened

private:
  size_t h1(int key) const;
  size_t h2(int key) const;

  // Toy: single vector of optional slots, two logical tables.
};
