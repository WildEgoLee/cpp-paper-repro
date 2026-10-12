// Minimal STM interfaces.
// Versioned locations + per-transaction read/write sets.

#pragma once
#include <cstdint>
#include <unordered_map>
#include <optional>

class STM;

class Transaction {
public:
  explicit Transaction(STM& stm);

  int read(int* addr);
  void write(int* addr, int value);
  bool commit();   // returns false on conflict (caller should retry)
  void abort();

private:
  friend class STM;
  STM& stm_;
  std::unordered_map<int*, int> read_set_;    // addr → version seen
  std::unordered_map<int*, int> write_set_;   // addr → new value
  bool active_ = true;
};

class STM {
public:
  // Shared locations are just ints; version tracked separately.
  int* alloc(int initial = 0);

private:
  friend class Transaction;
  struct Loc {
    int value = 0;
    uint64_t version = 0;
  };
  // In the toy, addresses are indices into a vector.
};
