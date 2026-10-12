// Minimal Epoch-based Reclamation interfaces.
// Global epoch + per-thread announce + 3 limbo lists.

#pragma once
#include <cstdint>
#include <vector>

class EBR {
public:
  // Enter / leave a critical section.
  void enter();
  void exit();

  // Retire a node; it will be freed after 2 epoch advances.
  void retire(void* node);

  // Try to advance the global epoch if all threads have observed it.
  void try_advance();

  uint64_t global_epoch() const;
  size_t limbo_size(int epoch) const;

private:
  uint64_t global_epoch_ = 0;
  // Per-thread announced epoch (toy: single thread).
  uint64_t announced_ = 0;
  bool in_critical_ = false;
  std::vector<void*> limbo_[3];
};
