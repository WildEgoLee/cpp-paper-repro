// Minimal Cuckoo Hashing.
// Two logical tables, two hash functions, kick-out on insert.
// Lookup is worst-case two probes.

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
      // Kick out the occupant.
      int kicked = *slot;
      slot = x;
      x = kicked;
      table = 1 - table;
      ++kicks_;
    }
    return false;   // cycle; rehash would be needed
  }

  bool contains(int key) const {
    auto a = t1_[h1(key)];
    if (a && *a == key) return true;
    auto b = t2_[h2(key)];
    return b && *b == key;
  }

  size_t kicks() const { return kicks_; }

private:
  size_t h1(int key) const {
    return static_cast<unsigned>(key) * 2654435761u % t1_.size();
  }
  size_t h2(int key) const {
    return static_cast<unsigned>(key) * 0x9e3779b9u % t2_.size();
  }

  std::vector<std::optional<int>> t1_, t2_;
  size_t kicks_;
};
