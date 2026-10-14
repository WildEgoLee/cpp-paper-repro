// Minimal Cuckoo Hashing.
// Two logical tables, two hash functions, kick-out on insert.
// Lookup is worst-case two probes.
// Hash functions are deliberately simple so collisions (and kicks) happen.

#pragma once
#include <vector>
#include <optional>
#include <cstddef>

class CuckooHash {
public:
  explicit CuckooHash(size_t capacity)
      : t1_(capacity), t2_(capacity), kicks_(0) {}

  bool insert(int key) {
    if (contains(key)) return true;
    int x = key;
    int table = 0;
    for (int i = 0; i < 32; ++i) {
      auto& slot = (table == 0) ? t1_[h1(x)] : t2_[h2(x)];
      if (!slot) {
        slot = x;
        return true;
      }
      int kicked = *slot;
      slot = x;
      x = kicked;
      table = 1 - table;
      ++kicks_;
    }
    return false;
  }

  bool contains(int key) const {
    auto a = t1_[h1(key)];
    if (a && *a == key) return true;
    auto b = t2_[h2(key)];
    return b && *b == key;
  }

  size_t kicks() const { return kicks_; }

private:
  // Simple modular hashes so small keys collide.
  size_t h1(int key) const { return static_cast<unsigned>(key) % t1_.size(); }
  size_t h2(int key) const { return static_cast<unsigned>(key * 3 + 1) % t2_.size(); }

  std::vector<std::optional<int>> t1_, t2_;
  size_t kicks_;
};
