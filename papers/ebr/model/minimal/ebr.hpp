// Minimal Epoch-based Reclamation.
// Single-thread toy that demonstrates the two-epoch rule
// and the stall (unbounded limbo) limitation.
//
// A node retired while the thread observed epoch e is freeable
// only after the global epoch has advanced to at least e+2.
// While the thread remains in a critical section, advance is blocked
// (toy approximation of "someone is still observing the old epoch").

#pragma once
#include <cstdint>
#include <vector>
#include <algorithm>

class EBR {
public:
  void enter() {
    in_critical_ = true;
    announced_ = global_epoch_;
  }

  void exit() {
    in_critical_ = false;
  }

  void retire(void* node) {
    uint64_t e = announced_;
    limbo_[e % 3].push_back({node, e});
  }

  bool try_advance() {
    // Toy: if we are still inside a critical section we have not
    // observed a newer epoch, so we cannot advance.
    if (in_critical_) return false;
    ++global_epoch_;
    if (global_epoch_ >= 2) {
      reclaim_up_to(global_epoch_ - 2);
    }
    return true;
  }

  uint64_t global_epoch() const { return global_epoch_; }
  uint64_t announced() const { return announced_; }
  bool in_critical() const { return in_critical_; }

  size_t limbo_count() const {
    size_t n = 0;
    for (auto& b : limbo_) n += b.size();
    return n;
  }

  size_t reclaimed_count() const { return reclaimed_; }

private:
  struct Item { void* ptr; uint64_t epoch; };

  void reclaim_up_to(uint64_t max_epoch) {
    for (auto& bag : limbo_) {
      auto it = std::remove_if(bag.begin(), bag.end(),
        [&](const Item& it) {
          if (it.epoch <= max_epoch) {
            ++reclaimed_;
            return true;
          }
          return false;
        });
      bag.erase(it, bag.end());
    }
  }

  uint64_t global_epoch_ = 0;
  uint64_t announced_ = 0;
  bool in_critical_ = false;
  std::vector<Item> limbo_[3];
  size_t reclaimed_ = 0;
};
