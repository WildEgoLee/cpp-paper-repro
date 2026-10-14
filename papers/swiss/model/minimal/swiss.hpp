// Minimal Swiss Table.
// Control bytes separate from keys; group probing with fingerprint filter.

#pragma once
#include <vector>
#include <optional>
#include <cstdint>
#include <cstddef>

class SwissTable {
public:
  explicit SwissTable(size_t capacity)
      : ctrl_(capacity, kEmpty), slots_(capacity), scans_(0) {}

  void insert(int key) {
    if (contains(key)) return;
    uint8_t h2 = fingerprint(key);
    size_t start = home(key);
    for (size_t g = 0; g < ctrl_.size(); ++g) {
      size_t i = (start + g) % ctrl_.size();
      if (ctrl_[i] == kEmpty || ctrl_[i] == kDeleted) {
        ctrl_[i] = h2;
        slots_[i] = key;
        return;
      }
    }
  }

  bool contains(int key) const {
    uint8_t h2 = fingerprint(key);
    size_t start = home(key);
    for (size_t g = 0; g < ctrl_.size(); ++g) {
      ++scans_;
      size_t i = (start + g) % ctrl_.size();
      uint8_t c = ctrl_[i];
      if (c == kEmpty) return false;
      if (c == h2 && slots_[i] && *slots_[i] == key) return true;
    }
    return false;
  }

  size_t groups_scanned() const { return scans_; }

private:
  static constexpr uint8_t kEmpty = 0xFF;
  static constexpr uint8_t kDeleted = 0x80;

  uint8_t fingerprint(int key) const {
    return static_cast<uint8_t>(static_cast<unsigned>(key) & 0x7F);
  }
  size_t home(int key) const {
    return static_cast<unsigned>(key) % ctrl_.size();
  }

  std::vector<uint8_t> ctrl_;
  std::vector<std::optional<int>> slots_;
  mutable size_t scans_;
};
