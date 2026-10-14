// Minimal Robin Hood Hashing interfaces.
// Linear probing + PSL comparison; richer keys are stolen.

#pragma once
#include <optional>

class RobinHood {
public:
  explicit RobinHood(size_t capacity);

  void insert(int key);
  bool contains(int key) const;
  // Maximum PSL currently in the table (used by the probe).
  int max_psl() const;

private:
  size_t home(int key) const;
};
