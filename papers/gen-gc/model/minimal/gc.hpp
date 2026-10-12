// Minimal generational GC (toy).
// Tracks nursery vs old counts and card-table dirty flags.
// No real object graph; demonstrates the protocol.

#pragma once
#include <vector>
#include <cstddef>

class GenGC {
public:
  GenGC() = default;

  // Allocate in nursery. Triggers minor GC when full.
  int alloc() {
    if (nursery_count_ >= kNurseryLimit) minor_gc();
    int id = next_id_++;
    nursery_ids_.push_back(id);
    ++nursery_count_;
    return id;
  }

  // Simulate an old→young pointer write. Marks a card dirty.
  void write_old_to_young() {
    if (old_count_ > 0) {
      card_.push_back(1);
      ++dirty_;
    }
  }

  // Minor GC: promote a fraction of nursery, clear the rest, reset cards.
  void minor_gc() {
    size_t promote = nursery_count_ / 2;   // toy policy: promote half
    old_count_ += promote;
    nursery_count_ = 0;
    nursery_ids_.clear();
    card_.clear();
    dirty_ = 0;
    ++minor_count_;
  }

  size_t nursery_used() const { return nursery_count_; }
  size_t old_used() const { return old_count_; }
  size_t dirty_cards() const { return dirty_; }
  size_t minor_gcs() const { return minor_count_; }

private:
  static constexpr size_t kNurseryLimit = 4;
  size_t nursery_count_ = 0;
  size_t old_count_ = 0;
  size_t dirty_ = 0;
  size_t minor_count_ = 0;
  int next_id_ = 0;
  std::vector<int> nursery_ids_;
  std::vector<char> card_;
};
