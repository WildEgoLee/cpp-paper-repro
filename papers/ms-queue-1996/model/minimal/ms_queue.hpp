// Minimal Michael-Scott lock-free queue with tagged-pointer ABA protection.
// On 64-bit we pack a 16-bit tag into the high bits of the pointer word
// (valid because user virtual addresses use at most 48 bits on x86-64).

#pragma once
#include <atomic>
#include <cstdint>
#include <optional>
#include <utility>

template<typename T>
class MSQueue {
  struct Node {
    T value;
    std::atomic<uintptr_t> next{0};   // tagged
    Node() = default;
    explicit Node(T v) : value(std::move(v)) {}
  };

  // Pack: low 48 bits = pointer, high 16 bits = tag
  static uintptr_t pack(Node* p, uint16_t tag) {
    return (static_cast<uintptr_t>(tag) << 48) |
           (reinterpret_cast<uintptr_t>(p) & 0x0000FFFFFFFFFFFFULL);
  }
  static Node* unpack_ptr(uintptr_t x) {
    uintptr_t p = x & 0x0000FFFFFFFFFFFFULL;
    // sign-extend if the original pointer was in the upper half (rare for heap)
    if (p & 0x0000800000000000ULL) p |= 0xFFFF000000000000ULL;
    return reinterpret_cast<Node*>(p);
  }
  static uint16_t unpack_tag(uintptr_t x) {
    return static_cast<uint16_t>(x >> 48);
  }

  std::atomic<uintptr_t> head_;
  std::atomic<uintptr_t> tail_;

public:
  MSQueue() {
    Node* dummy = new Node();
    head_.store(pack(dummy, 0), std::memory_order_relaxed);
    tail_.store(pack(dummy, 0), std::memory_order_relaxed);
  }

  ~MSQueue() {
    while (dequeue()) {}
    delete unpack_ptr(head_.load(std::memory_order_relaxed));
  }

  void enqueue(T value) {
    Node* node = new Node(std::move(value));
    while (true) {
      uintptr_t tail = tail_.load(std::memory_order_acquire);
      Node* tp = unpack_ptr(tail);
      uintptr_t next = tp->next.load(std::memory_order_acquire);
      if (tail != tail_.load(std::memory_order_acquire)) continue;

      if (unpack_ptr(next) == nullptr) {
        uintptr_t new_next = pack(node, unpack_tag(next) + 1);
        if (tp->next.compare_exchange_weak(next, new_next,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
          tail_.compare_exchange_strong(tail, pack(node, unpack_tag(tail) + 1),
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
          return;
        }
      } else {
        tail_.compare_exchange_strong(tail,
                                      pack(unpack_ptr(next), unpack_tag(tail) + 1),
                                      std::memory_order_release,
                                      std::memory_order_relaxed);
      }
    }
  }

  std::optional<T> dequeue() {
    while (true) {
      uintptr_t head = head_.load(std::memory_order_acquire);
      uintptr_t tail = tail_.load(std::memory_order_acquire);
      Node* hp = unpack_ptr(head);
      uintptr_t next_raw = hp->next.load(std::memory_order_acquire);
      Node* next = unpack_ptr(next_raw);
      if (head != head_.load(std::memory_order_acquire)) continue;

      if (next == nullptr) return std::nullopt;

      if (hp == unpack_ptr(tail)) {
        tail_.compare_exchange_strong(tail,
                                      pack(next, unpack_tag(tail) + 1),
                                      std::memory_order_release,
                                      std::memory_order_relaxed);
        continue;
      }

      T value = next->value;
      if (head_.compare_exchange_strong(head,
                                        pack(next, unpack_tag(head) + 1),
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
        delete hp;
        return value;
      }
    }
  }
};
