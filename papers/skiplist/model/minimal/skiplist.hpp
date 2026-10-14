// Minimal Skip List interfaces.
// Random height + multi-level forward pointers.

#pragma once
#include <optional>

class SkipList {
public:
  void insert(int key);
  bool contains(int key) const;
  // Highest level currently in use.
  int max_level() const;

private:
  static constexpr int kMaxLevel = 8;
  static constexpr double kP = 0.5;

  struct Node {
    int key;
    Node* forward[kMaxLevel];
    int level;
  };
  Node* head_ = nullptr;
  int level_ = 0;
};
