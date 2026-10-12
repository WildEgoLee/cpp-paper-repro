// Minimal Actor model interfaces.

#pragma once
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
#include <any>

class Actor {
public:
  using Behavior = std::function<void(Actor& self, std::any msg)>;

  explicit Actor(Behavior b);

  void send(std::any msg);
  void become(Behavior b);
  void process_one();   // run one message if available

private:
  Behavior behavior_;
  std::queue<std::any> mailbox_;
  std::mutex mu_;
};
