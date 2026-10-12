// Minimal counter-based RCU implementation for learning.
// Not production-quality; demonstrates grace period idea.

#include "rcu.hpp"
#include <algorithm>
#include <chrono>
#include <mutex>
#include <thread>
#include <vector>

struct Pending {
  uint64_t gp;
  std::function<void()> cb;
};

class CounterRcuImpl : public CounterRcu {
public:
  void read_lock(ThreadRcuState& tls) override {
    int n = tls.nesting.fetch_add(1, std::memory_order_acq_rel);
    if (n == 0) {
      tls.gp_snapshot.store(gp_.load(std::memory_order_acquire), std::memory_order_release);
    }
  }

  void read_unlock(ThreadRcuState& tls) override {
    tls.nesting.fetch_sub(1, std::memory_order_acq_rel);
  }

  void synchronize() override {
    uint64_t target = gp_.fetch_add(1, std::memory_order_acq_rel) + 1;
    // Toy: coarse wait. Real RCU scans a thread registry.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    drain_callbacks(target);
  }

  void call_rcu(std::function<void()> cb) override {
    std::lock_guard<std::mutex> g(mu_);
    pending_.push_back({gp_.load(std::memory_order_acquire), std::move(cb)});
  }

private:
  void drain_callbacks(uint64_t up_to) {
    std::lock_guard<std::mutex> g(mu_);
    auto it = std::remove_if(pending_.begin(), pending_.end(),
      [up_to](Pending& p) {
        if (p.gp < up_to) { p.cb(); return true; }
        return false;
      });
    pending_.erase(it, pending_.end());
  }

  std::atomic<uint64_t> gp_{0};
  std::mutex mu_;
  std::vector<Pending> pending_;
};

CounterRcu* make_counter_rcu() { return new CounterRcuImpl(); }
