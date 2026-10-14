// Cuckoo Hashing with rehash.
// Two tables, two hash functions, kick-out on insert.
// If the kick chain exceeds a limit, rebuild with larger tables.

#pragma once
#include <vector>
#include <optional>
#include <cstddef>

class CuckooHash {
public:
  explicit CuckooHash(size_t capacity)
      : t1_(capacity), t2_(capacity), kicks_(0), rehashes_(0) {}

  bool insert(int key) {
    if (contains(key)) return true;
    if (!try_insert(key)) {
      rehash();
      return try_insert(key);
    }
    return true;
  }

  bool contains(int key) const {
    auto a = t1_[h1(key)];
    if (a && *a == key) return true;
    auto b = t2_[h2(key)];
    return b && *b == key;
  }

  size_t kicks() const { return kicks_; }
  size_t rehashes() const { return rehashes_; }
  size_t capacity() const { return t1_.size(); }

private:
  bool try_insert(int key) {
    int x = key;
    int table = 0;
    for (int i = 0; i < 16; ++i) {
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

  void rehash() {
    auto old1 = std::move(t1_);
    auto old2 = std::move(t2_);
    size_t n = old1.size() * 2;
    t1_.assign(n, std::nullopt);
    t2_.assign(n, std::nullopt);
    ++rehashes_;
    for (auto& s : old1) if (s) try_insert(*s);
    for (auto& s : old2) if (s) try_insert(*s);
  }

  size_t h1(int key) const { return static_cast<unsigned>(key) % t1_.size(); }
  size_t h2(int key) const { return static_cast<unsigned>(key * 3 + 1) % t2_.size(); }

  std::vector<std::optional<int>> t1_, t2_;
  size_t kicks_;
  size_t rehashes_;
};
