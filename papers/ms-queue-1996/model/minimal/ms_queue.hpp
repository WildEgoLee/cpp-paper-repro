// Minimal Michael-Scott lock-free queue.
// Dummy node + two-step CAS. Header-only for simplicity.

#pragma once
#include <atomic>
#include <optional>
#include <utility>

template<typename T>
class MSQueue {
  struct Node {
    T value;
    std::atomic<Node*> next{nullptr};
    Node() = default;
    explicit Node(T v) : value(std::move(v)) {}
  };

  std::atomic<Node*> head_;
  std::atomic<Node*> tail_;

public:
  MSQueue() {
    Node* dummy = new Node();
    head_.store(dummy, std::memory_order_relaxed);
    tail_.store(dummy, std::memory_order_relaxed);
  }

  ~MSQueue() {
    while (dequeue()) {}
    delete head_.load(std::memory_order_relaxed);
  }

  void enqueue(T value) {
    Node* node = new Node(std::move(value));
    while (true) {
      Node* tail = tail_.load(std::memory_order_acquire);
      Node* next = tail->next.load(std::memory_order_acquire);
      if (tail != tail_.load(std::memory_order_acquire)) continue;
      if (next == nullptr) {
        if (tail->next.compare_exchange_weak(next, node,
                                             std::memory_order_release,
                                             std::memory_order_relaxed)) {
          tail_.compare_exchange_strong(tail, node,
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
          return;
        }
      } else {
        tail_.compare_exchange_strong(tail, next,
                                      std::memory_order_release,
                                      std::memory_order_relaxed);
      }
    }
  }

  std::optional<T> dequeue() {
    while (true) {
      Node* head = head_.load(std::memory_order_acquire);
      Node* tail = tail_.load(std::memory_order_acquire);
      Node* next = head->next.load(std::memory_order_acquire);
      if (head != head_.load(std::memory_order_acquire)) continue;
      if (next == nullptr) return std::nullopt;
      if (head == tail) {
        tail_.compare_exchange_strong(tail, next,
                                      std::memory_order_release,
                                      std::memory_order_relaxed);
        continue;
      }
      T value = next->value;
      if (head_.compare_exchange_strong(head, next,
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
        delete head;
        return value;
      }
    }
  }
};
