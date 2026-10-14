// Cuckoo Hashing with rehash.
// Two tables, two hash functions, kick-out on insert.
// If the kick chain exceeds a limit, rebuild with larger tables.
// During rehash we place keys into empty slots to avoid losing them.

#pragma once
#include <vector>
#include <optional>
#include <cstddef>
#include <algorithm>

class CuckooHash {
public:
  explicit CuckooHash(size_t capacity)
      : t1_(capacity), t2_(capacity), kicks_(0), rehashes_(0) {}

  bool insert(int key) {
    if (contains(key)) return true;
    for (int attempt = 0; attempt < 8; ++attempt) {
      if (try_insert(key)) return true;
      rehash();
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
  size_t rehashes() const { return rehashes_; }
  size_t capacity() const { return t1_.size(); }

private:
  bool try_insert(int key) {
    int x = key;
    int table = 0;
    for (int i = 0; i < 64; ++i) {
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
    std::vector<int> keys;
    for (auto& s : t1_) if (s) keys.push_back(*s);
    for (auto& s : t2_) if (s) keys.push_back(*s);
    size_t n = std::max(t1_.size() * 4, keys.size() * 16 + 16);
    t1_.assign(n, std::nullopt);
    t2_.assign(n, std::nullopt);
    ++rehashes_;
    // Place each key into an empty preferred slot; fall back to any empty.
    for (int k : keys) {
      if (!t1_[h1(k)]) t1_[h1(k)] = k;
      else if (!t2_[h2(k)]) t2_[h2(k)] = k;
      else {
        // Find any empty slot in t1.
        for (size_t i = 0; i < n; ++i) {
          if (!t1_[i]) { t1_[i] = k; break; }
        }
      }
    }
  }

  size_t h1(int key) const { return static_cast<unsigned>(key) % t1_.size(); }
  size_t h2(int key) const { return static_cast<unsigned>(key * 13 + 7) % t2_.size(); }

  std::vector<std::optional<int>> t1_, t2_;
  size_t kicks_;
  size_t rehashes_;
};
