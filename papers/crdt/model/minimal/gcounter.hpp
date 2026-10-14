// Minimal G-Counter (state-based CRDT).
// One slot per replica; merge is pointwise max; value is sum.
// merge is commutative, associative, and idempotent.

#pragma once
#include <vector>
#include <algorithm>
#include <cstddef>

class GCounter {
public:
  GCounter(size_t n_replicas, size_t my_id)
      : payload_(n_replicas, 0), my_id_(my_id) {}

  void increment(int k = 1) { payload_[my_id_] += k; }

  void merge(const GCounter& other) {
    for (size_t i = 0; i < payload_.size(); ++i)
      payload_[i] = std::max(payload_[i], other.payload_[i]);
  }

  int value() const {
    int s = 0;
    for (int x : payload_) s += x;
    return s;
  }

  bool same_state(const GCounter& other) const {
    return payload_ == other.payload_;
  }

private:
  std::vector<int> payload_;
  size_t my_id_;
};
