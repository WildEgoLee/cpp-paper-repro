// Minimal Epoch-based Reclamation.
// Single-thread toy that demonstrates the two-epoch rule.
// A node retired in epoch e is freeable only after the global epoch
// has advanced to e+2.

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
    // Retire into the limbo list of the epoch the thread observed.
    uint64_t e = announced_;
    limbo_[e % 3].push_back({node, e});
  }

  // Advance if the (single) thread has observed the current epoch
  // or is not in a critical section.
  bool try_advance() {
    if (in_critical_ && announced_ < global_epoch_) return false;
    ++global_epoch_;
    // Free anything retired in epoch (global - 2).
    uint64_t freeable = global_epoch_ >= 2 ? global_epoch_ - 2 : ~0ull;
    reclaim(freeable);
    return true;
  }

  uint64_t global_epoch() const { return global_epoch_; }
  uint64_t announced() const { return announced_; }

  // How many nodes are still in limbo (not yet freeable).
  size_t limbo_count() const {
    size_t n = 0;
    for (auto& b : limbo_) n += b.size();
    return n;
  }

  // How many have been reclaimed.
  size_t reclaimed_count() const { return reclaimed_; }

private:
  struct Item { void* ptr; uint64_t epoch; };

  void reclaim(uint64_t freeable_epoch) {
    for (auto& bag : limbo_) {
      auto it = std::remove_if(bag.begin(), bag.end(),
        [&](const Item& it) {
          if (it.epoch <= freeable_epoch) {
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
