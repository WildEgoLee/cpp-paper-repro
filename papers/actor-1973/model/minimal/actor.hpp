// Minimal Actor model.
// Private mailbox + behavior. Header-only.

#pragma once
#include <any>
#include <functional>
#include <mutex>
#include <queue>
#include <utility>

class Actor {
public:
  using Behavior = std::function<void(Actor& self, std::any msg)>;

  explicit Actor(Behavior b) : behavior_(std::move(b)) {}

  void send(std::any msg) {
    std::lock_guard<std::mutex> lk(mu_);
    mailbox_.push(std::move(msg));
  }

  void become(Behavior b) {
    behavior_ = std::move(b);
  }

  // Process one message if available. Returns false if mailbox empty.
  bool process_one() {
    std::any msg;
    {
      std::lock_guard<std::mutex> lk(mu_);
      if (mailbox_.empty()) return false;
      msg = std::move(mailbox_.front());
      mailbox_.pop();
    }
    if (behavior_) behavior_(*this, std::move(msg));
    return true;
  }

private:
  Behavior behavior_;
  std::queue<std::any> mailbox_;
  std::mutex mu_;
};
