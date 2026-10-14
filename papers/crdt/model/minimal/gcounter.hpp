// Minimal G-Counter (state-based CRDT).
// One slot per replica; merge is pointwise max; value is sum.

#pragma once
#include <vector>
#include <cstddef>

class GCounter {
public:
  explicit GCounter(size_t n_replicas, size_t my_id);

  void increment();
  void merge(const GCounter& other);
  int value() const;

private:
  std::vector<int> payload_;
  size_t my_id_;
};
