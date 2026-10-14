// Minimal B+ Tree.
// Toy: a single sorted leaf chain. Demonstrates the leaf-list
// property that makes range scans O(K) after the first leaf.
// No internal nodes yet; the architecture point is the leaf chain.

#pragma once
#include <vector>
#include <optional>
#include <algorithm>

class BPTree {
public:
  ~BPTree() {
    Leaf* p = head_;
    while (p) { Leaf* n = p->next; delete p; p = n; }
  }

  void insert(int key, int value) {
    if (!head_) {
      head_ = new Leaf;
      head_->entries.push_back({key, value});
      return;
    }
    Leaf* leaf = head_;
    while (leaf->next && leaf->next->entries.front().first <= key)
      leaf = leaf->next;

    auto it = std::lower_bound(leaf->entries.begin(), leaf->entries.end(),
                               std::make_pair(key, 0),
                               [](auto a, auto b) { return a.first < b.first; });
    leaf->entries.insert(it, {key, value});

    if (leaf->entries.size() > kCapacity) {
      auto* right = new Leaf;
      size_t mid = leaf->entries.size() / 2;
      right->entries.assign(leaf->entries.begin() + mid, leaf->entries.end());
      leaf->entries.resize(mid);
      right->next = leaf->next;
      leaf->next = right;
    }
  }

  std::optional<int> lookup(int key) const {
    for (Leaf* p = head_; p; p = p->next) {
      for (auto& e : p->entries)
        if (e.first == key) return e.second;
      if (!p->entries.empty() && p->entries.back().first > key) break;
    }
    return std::nullopt;
  }

  std::vector<int> range(int lo, int hi) const {
    std::vector<int> out;
    for (Leaf* p = head_; p; p = p->next) {
      for (auto& e : p->entries) {
        if (e.first > hi) return out;
        if (e.first >= lo) out.push_back(e.second);
      }
    }
    return out;
  }

  size_t leaf_count() const {
    size_t n = 0;
    for (Leaf* p = head_; p; p = p->next) ++n;
    return n;
  }

private:
  static constexpr size_t kCapacity = 3;
  struct Leaf {
    std::vector<std::pair<int, int>> entries;
    Leaf* next = nullptr;
  };
  Leaf* head_ = nullptr;
};
