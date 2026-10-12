// Minimal io_uring-style ring interfaces (user-space simulation).
// No real kernel; demonstrates SQ/CQ head-tail protocol.

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
  explicit Ring(size_t n);

  // Producer
  bool try_push(const T& item);
  void push(const T& item);          // may spin

  // Consumer
  std::optional<T> try_pop();

  size_t size() const;

private:
  std::vector<T> buf_;
  std::atomic<uint32_t> head_{0};
  std::atomic<uint32_t> tail_{0};
  uint32_t mask_;
};

using SubmissionQueue = Ring<SQE>;
using CompletionQueue = Ring<CQE>;
