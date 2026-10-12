// Minimal generational GC interfaces.
// Nursery + old + card-table write barrier.

#pragma once
#include <cstdint>
#include <vector>

class GenGC {
public:
  // Allocate in nursery. Triggers minor GC if full.
  void* alloc(size_t size);

  // Mutator write of a pointer. Write barrier records old→young.
  void write_ptr(void** slot, void* value);

  // Collect nursery; promote survivors.
  void minor_gc();

  size_t nursery_used() const;
  size_t old_used() const;
  size_t dirty_cards() const;

private:
  std::vector<char> nursery_;
  std::vector<char> old_;
  std::vector<bool> card_table_;   // dirty flags
  // Remembered set derived from dirty cards.
};
