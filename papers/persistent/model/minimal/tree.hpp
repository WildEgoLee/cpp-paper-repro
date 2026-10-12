// Minimal persistent binary search tree interfaces.
// Path copying on update; old roots remain valid.

#pragma once
#include <memory>

struct Node {
  int key;
  int value;
  std::shared_ptr<Node> left;
  std::shared_ptr<Node> right;

  Node(int k, int v,
       std::shared_ptr<Node> l = nullptr,
       std::shared_ptr<Node> r = nullptr)
    : key(k), value(v), left(std::move(l)), right(std::move(r)) {}
};

using Root = std::shared_ptr<Node>;

// Returns a new root; the old root is unchanged.
Root insert(Root root, int key, int value);

// Lookup in a given version.
int lookup(Root root, int key);
