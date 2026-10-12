// Minimal fiber interfaces.
// First version uses explicit yield points + a simple scheduler.
// Real stack switching would use ucontext or assembly.

#pragma once
#include <functional>
#include <vector>
#include <cstddef>

class Scheduler;

class Fiber {
public:
  using Fn = std::function<void(Fiber&)>;

  Fiber(Scheduler& sched, Fn fn);

  void yield();
  bool done() const;

private:
  friend class Scheduler;
  Scheduler& sched_;
  Fn fn_;
  bool started_ = false;
  bool done_ = false;
  // In a real impl: void* stack; ucontext_t ctx;
};

class Scheduler {
public:
  Fiber* spawn(Fiber::Fn fn);
  void run();

private:
  friend class Fiber;
  std::vector<Fiber*> ready_;
  Fiber* current_ = nullptr;
};
