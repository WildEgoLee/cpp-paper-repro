// Minimal Skip List.
// Random height + multi-level forward pointers.
// Search drops down when the next key is too big.

#pragma once
#include <cstdlib>
#include <cstring>

class SkipList {
public:
  SkipList() {
    head_ = new Node;
    head_->key = -1;
    head_->level = kMaxLevel;
    for (int i = 0; i < kMaxLevel; ++i) head_->forward[i] = nullptr;
    level_ = 1;
  }

  ~SkipList() {
    Node* p = head_;
    while (p) { Node* n = p->forward[0]; delete p; p = n; }
  }

  void insert(int key) {
    Node* update[kMaxLevel];
    Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
      while (x->forward[i] && x->forward[i]->key < key)
        x = x->forward[i];
      update[i] = x;
    }
    int h = random_level();
    if (h > level_) {
      for (int i = level_; i < h; ++i) update[i] = head_;
      level_ = h;
    }
    Node* n = new Node;
    n->key = key;
    n->level = h;
    for (int i = 0; i < h; ++i) {
      n->forward[i] = update[i]->forward[i];
      update[i]->forward[i] = n;
    }
  }

  bool contains(int key) const {
    Node* x = head_;
    for (int i = level_ - 1; i >= 0; --i) {
      while (x->forward[i] && x->forward[i]->key < key)
        x = x->forward[i];
    }
    x = x->forward[0];
    return x && x->key == key;
  }

  int max_level() const { return level_; }

  // How many nodes have height >= 2 (used by the probe).
  int tall_nodes() const {
    int n = 0;
    for (Node* p = head_->forward[0]; p; p = p->forward[0])
      if (p->level >= 2) ++n;
    return n;
  }

private:
  static constexpr int kMaxLevel = 8;

  struct Node {
    int key;
    Node* forward[kMaxLevel];
    int level;
  };

  int random_level() {
    int h = 1;
    while (h < kMaxLevel && (std::rand() & 1) == 0) ++h;
    return h;
  }

  Node* head_;
  int level_;
};
