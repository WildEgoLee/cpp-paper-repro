// Minimal RCU model — interfaces for learning architecture.
// Not a production implementation.

#pragma once
#include <atomic>
#include <cstdint>
#include <functional>
#include <vector>

// Per-thread state
struct ThreadRcuState {
  std::atomic<int> nesting{0};          // reader critical section depth
  std::atomic<uint64_t> gp_snapshot{0}; // grace period seen when entering
};

// Domain that tracks grace periods
class RcuDomain {
public:
  virtual ~RcuDomain() = default;

  // Reader side (fast path)
  virtual void read_lock(ThreadRcuState& tls) = 0;
  virtual void read_unlock(ThreadRcuState& tls) = 0;

  // Writer side
  virtual void synchronize() = 0;  // wait for grace period

  // Deferred free
  virtual void call_rcu(std::function<void()> cb) = 0;
};

// Simple counter-based toy (to be implemented)
class CounterRcu : public RcuDomain {
public:
  void read_lock(ThreadRcuState& tls) override;
  void read_unlock(ThreadRcuState& tls) override;
  void synchronize() override;
  void call_rcu(std::function<void()> cb) override;

private:
  std::atomic<uint64_t> gp_{0};
  // pending callbacks, etc.
};
