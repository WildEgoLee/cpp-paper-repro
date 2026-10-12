// Minimal io_uring-style ring (user-space simulation).
// SPSC ring demonstrating SQ/CQ head-tail protocol.

#pragma once
#include <atomic>
#include <cstdint>
#include <optional>
#include <vector>

struct SQE {
  uint8_t opcode = 0;
  int fd = -1;
  uint64_t addr = 0;
  uint32_t len = 0;
  uint64_t user_data = 0;
};

struct CQE {
  uint64_t user_data = 0;
  int32_t res = 0;
  uint32_t flags = 0;
};

template<typename T>
class Ring {
public:
  explicit Ring(size_t n) : buf_(n), mask_(static_cast<uint32_t>(n - 1)) {
    // n must be power of two for mask to work; we assume caller guarantees it
  }

  bool try_push(const T& item) {
    uint32_t t = tail_.load(std::memory_order_relaxed);
    uint32_t h = head_.load(std::memory_order_acquire);
    if (t - h >= buf_.size()) return false;  // full
    buf_[t & mask_] = item;
    tail_.store(t + 1, std::memory_order_release);
    return true;
  }

  std::optional<T> try_pop() {
    uint32_t h = head_.load(std::memory_order_relaxed);
    uint32_t t = tail_.load(std::memory_order_acquire);
    if (h == t) return std::nullopt;  // empty
    T item = buf_[h & mask_];
    head_.store(h + 1, std::memory_order_release);
    return item;
  }

  size_t size() const {
    return tail_.load(std::memory_order_acquire) - head_.load(std::memory_order_acquire);
  }

private:
  std::vector<T> buf_;
  std::atomic<uint32_t> head_{0};
  std::atomic<uint32_t> tail_{0};
  uint32_t mask_;
};

using SubmissionQueue = Ring<SQE>;
using CompletionQueue = Ring<CQE>;
