// Michael-Scott queue with tagged pointers + hazard pointer reclamation.

#pragma once
#include "hazard_pointer.hpp"
#include <atomic>
#include <cstdint>
#include <optional>
#include <utility>

template<typename T>
class MSQueue {
  struct Node {
    T value;
    std::atomic<uintptr_t> next{0};
    Node() = default;
    explicit Node(T v) : value(std::move(v)) {}
  };

  static uintptr_t pack(Node* p, uint16_t tag) {
    return (static_cast<uintptr_t>(tag) << 48) |
           (reinterpret_cast<uintptr_t>(p) & 0x0000FFFFFFFFFFFFULL);
  }
  static Node* unpack_ptr(uintptr_t x) {
    uintptr_t p = x & 0x0000FFFFFFFFFFFFULL;
    if (p & 0x0000800000000000ULL) p |= 0xFFFF000000000000ULL;
    return reinterpret_cast<Node*>(p);
  }
  static uint16_t unpack_tag(uintptr_t x) { return static_cast<uint16_t>(x >> 48); }

  static void node_deleter(void* p) { delete static_cast<Node*>(p); }

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
    auto& hp = HazardPointerDomain::instance();
    HPRecord* rec = hp.acquire();

    while (true) {
      uintptr_t tail = tail_.load(std::memory_order_acquire);
      Node* tp = unpack_ptr(tail);
      rec->slots[0].store(tp, std::memory_order_release);   // protect tail
      if (tail != tail_.load(std::memory_order_acquire)) continue;

      uintptr_t next = tp->next.load(std::memory_order_acquire);
      if (unpack_ptr(next) == nullptr) {
        uintptr_t new_next = pack(node, unpack_tag(next) + 1);
        if (tp->next.compare_exchange_weak(next, new_next,
                                           std::memory_order_release,
                                           std::memory_order_relaxed)) {
          tail_.compare_exchange_strong(tail, pack(node, unpack_tag(tail) + 1),
                                        std::memory_order_release,
                                        std::memory_order_relaxed);
          rec->slots[0].store(nullptr, std::memory_order_release);
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
    auto& hp = HazardPointerDomain::instance();
    HPRecord* rec = hp.acquire();

    while (true) {
      uintptr_t head = head_.load(std::memory_order_acquire);
      Node* hp_node = unpack_ptr(head);
      rec->slots[0].store(hp_node, std::memory_order_release);  // protect head
      if (head != head_.load(std::memory_order_acquire)) continue;

      uintptr_t tail = tail_.load(std::memory_order_acquire);
      uintptr_t next_raw = hp_node->next.load(std::memory_order_acquire);
      Node* next = unpack_ptr(next_raw);

      if (next == nullptr) {
        rec->slots[0].store(nullptr, std::memory_order_release);
        return std::nullopt;
      }

      if (hp_node == unpack_ptr(tail)) {
        tail_.compare_exchange_strong(tail,
                                      pack(next, unpack_tag(tail) + 1),
                                      std::memory_order_release,
                                      std::memory_order_relaxed);
        continue;
      }

      rec->slots[1].store(next, std::memory_order_release);   // protect next
      T value = next->value;

      if (head_.compare_exchange_strong(head,
                                        pack(next, unpack_tag(head) + 1),
                                        std::memory_order_release,
                                        std::memory_order_relaxed)) {
        rec->slots[0].store(nullptr, std::memory_order_release);
        rec->slots[1].store(nullptr, std::memory_order_release);
        hp.retire(hp_node, node_deleter);   // safe free later
        return value;
      }
    }
  }
};
