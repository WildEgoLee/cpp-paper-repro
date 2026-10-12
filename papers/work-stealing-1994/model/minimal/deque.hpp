// Minimal work-stealing deque.
// Owner pushes/pops at bottom (LIFO); thieves steal from top (FIFO).
// Toy uses a mutex; real impl uses CAS.

#pragma once
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

template<typename T>
class WorkStealingDeque {
public:
  void push_bottom(T item) {
    std::lock_guard<std::mutex> lk(mu_);
    q_.push_back(std::move(item));
  }

  std::optional<T> pop_bottom() {
    std::lock_guard<std::mutex> lk(mu_);
    if (q_.empty()) return std::nullopt;
    T item = std::move(q_.back());
    q_.pop_back();
    return item;
  }

  std::optional<T> steal_top() {
    std::lock_guard<std::mutex> lk(mu_);
    if (q_.empty()) return std::nullopt;
    T item = std::move(q_.front());
    q_.pop_front();
    return item;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lk(mu_);
    return q_.empty();
  }

private:
  std::deque<T> q_;
  mutable std::mutex mu_;
};
