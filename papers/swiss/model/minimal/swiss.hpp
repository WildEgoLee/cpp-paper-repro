// Minimal Swiss Table interfaces.
// Control bytes + group probing.

#pragma once
#include <optional>

class SwissTable {
public:
  explicit SwissTable(size_t capacity);

  void insert(int key);
  bool contains(int key) const;
  // How many control-byte groups were scanned (used by the probe).
  size_t groups_scanned() const;

private:
  static constexpr uint8_t kEmpty = 0xFF;
  static constexpr uint8_t kDeleted = 0x80;
};
