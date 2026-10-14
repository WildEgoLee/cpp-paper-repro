// Minimal Quotient Filter interfaces.
// Hash split into quotient (slot) and remainder (stored).

#pragma once
#include <cstddef>

class QuotientFilter {
public:
  explicit QuotientFilter(size_t slots);

  void insert(int key);
  bool contains(int key) const;
  bool remove(int key);

private:
  // slots_ store remainders
};
