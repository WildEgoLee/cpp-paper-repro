// Minimal Michael-Scott queue interfaces.
// Dummy node + two-step CAS.

#pragma once
#include <atomic>
#include <memory>
#include <optional>

template<typename T>
class MSQueue {
public:
  MSQueue();
  ~MSQueue();

  void enqueue(T value);
  std::optional<T> dequeue();

private:
  struct Node {
    T value;
    std::atomic<Node*> next{nullptr};
    Node() = default;
    explicit Node(T v) : value(std::move(v)) {}
  };

  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;
};
