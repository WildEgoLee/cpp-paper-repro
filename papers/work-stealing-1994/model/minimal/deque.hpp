// Minimal work-stealing deque interfaces.
// Owner operates on bottom; thieves on top.

#pragma once
#include <atomic>
#include <optional>
#include <vector>
#include <mutex>

template<typename T>
class WorkStealingDeque {
public:
  // Owner
  void push_bottom(T item);
  std::optional<T> pop_bottom();

  // Thief
  std::optional<T> steal_top();

  bool empty() const;

private:
  std::vector<T> buf_;
  std::atomic<int> top_{0};
  std::atomic<int> bottom_{0};
  mutable std::mutex mu_;   // toy; real impl uses CAS
};
