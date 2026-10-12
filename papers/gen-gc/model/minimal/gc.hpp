// Minimal generational GC.
// Nursery + old generation + card-table write barrier.
// Toy: objects are nodes with an optional next pointer.

#pragma once
#include <vector>
#include <cstddef>

struct GCObj {
  int id;
  int age = 0;
  GCObj* next = nullptr;
};

class GenGC {
public:
  GenGC() = default;
  ~GenGC() {
    for (auto* o : nursery_) delete o;
    for (auto* o : old_) delete o;
  }

  GCObj* alloc() {
    if (nursery_.size() >= kNurseryLimit) minor_gc();
    auto* o = new GCObj{next_id_++};
    nursery_.push_back(o);
    return o;
  }

  // Write barrier: if slot is in old and value is in nursery, mark card dirty.
  void write_ptr(GCObj* slot, GCObj* value) {
    slot->next = value;
    if (is_old(slot) && is_nursery(value)) {
      for (size_t i = 0; i < old_.size(); ++i)
        if (old_[i] == slot) card_[i] = 1;
    }
  }

  void minor_gc() {
    // Roots = remembered set (dirty cards pointing into nursery)
    std::vector<GCObj*> survivors;
    for (size_t i = 0; i < old_.size(); ++i) {
      if (card_[i] && old_[i]->next && is_nursery(old_[i]->next))
        survivors.push_back(old_[i]->next);
    }
    // Promote objects that have survived once
    for (auto* o : survivors) {
      if (o->age >= 1) {
        old_.push_back(o);
        card_.push_back(0);
        // remove from nursery list (toy: mark)
        o->age = 100;
      } else {
        ++o->age;
      }
    }
    // Drop dead nursery objects
    std::vector<GCObj*> keep;
    for (auto* o : nursery_) {
      if (o->age < 100) keep.push_back(o);
      else { /* promoted */ }
    }
    for (auto* o : nursery_)
      if (o->age >= 100) { /* already in old */ }
      else if (o->age == 0) delete o;   // never reached, dead
    nursery_ = std::move(keep);
    for (auto& c : card_) c = 0;
  }

  size_t nursery_used() const { return nursery_.size(); }
  size_t old_used() const { return old_.size(); }
  size_t dirty_cards() const {
    size_t n = 0;
    for (auto c : card_) if (c) ++n;
    return n;
  }

private:
  static constexpr size_t kNurseryLimit = 4;
  std::vector<GCObj*> nursery_;
  std::vector<GCObj*> old_;
  std::vector<char> card_;
  int next_id_ = 0;

  bool is_old(GCObj* o) const {
    for (auto* x : old_) if (x == o) return true;
    return false;
  }
  bool is_nursery(GCObj* o) const {
    for (auto* x : nursery_) if (x == o) return true;
    return false;
  }
};
