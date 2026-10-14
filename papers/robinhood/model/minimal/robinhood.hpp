// Minimal Robin Hood Hashing.
// Linear probing; on insert, steal from a richer key (smaller PSL).

#pragma once
#include <vector>
#include <optional>
#include <cstddef>

class RobinHood {
public:
  explicit RobinHood(size_t capacity) : slots_(capacity) {}

  void insert(int key) {
    if (contains(key)) return;
    size_t i = home(key);
    int psl = 0;
    int x = key;
    for (size_t steps = 0; steps < slots_.size(); ++steps) {
      auto& s = slots_[i];
      if (!s) {
        s = Slot{x, psl};
        return;
      }
      // Steal if the existing key is richer (smaller PSL).
      if (psl > s->psl) {
        std::swap(x, s->key);
        std::swap(psl, s->psl);
      }
      i = (i + 1) % slots_.size();
      ++psl;
    }
  }

  bool contains(int key) const {
    size_t i = home(key);
    int psl = 0;
    for (size_t steps = 0; steps < slots_.size(); ++steps) {
      auto& s = slots_[i];
      if (!s) return false;
      if (s->key == key) return true;
      // If we see a smaller PSL, the key cannot be further along.
      if (s->psl < psl) return false;
      i = (i + 1) % slots_.size();
      ++psl;
    }
    return false;
  }

  int max_psl() const {
    int m = 0;
    for (auto& s : slots_)
      if (s && s->psl > m) m = s->psl;
    return m;
  }

  size_t steals() const { return steals_; }

private:
  struct Slot { int key; int psl; };

  size_t home(int key) const {
    return static_cast<unsigned>(key) % slots_.size();
  }

  std::vector<std::optional<Slot>> slots_;
  size_t steals_ = 0;
};
