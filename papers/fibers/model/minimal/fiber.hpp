// Minimal cooperative fiber scheduler.
// Explicit yield points; no real stack switching (that would need ucontext/asm).
// The fiber body is driven step-by-step so yield returns control to the scheduler.

#pragma once
#include <functional>
#include <queue>
#include <vector>

class Scheduler;

class Fiber {
public:
  using Step = std::function<bool(Fiber&)>;  // returns true if more work

  Fiber(Scheduler& sched, Step step);

  // Called by the fiber body to give up the CPU.
  void yield();

  bool done() const { return done_; }

private:
  friend class Scheduler;
  Scheduler& sched_;
  Step step_;
  bool done_ = false;
  int yield_count_ = 0;
};

class Scheduler {
public:
  Fiber* spawn(Fiber::Step step);
  void run();

  int total_yields() const { return total_yields_; }

private:
  friend class Fiber;
  void requeue(Fiber* f);
  std::queue<Fiber*> ready_;
  int total_yields_ = 0;
};

inline Fiber::Fiber(Scheduler& sched, Step step)
  : sched_(sched), step_(std::move(step)) {}

inline void Fiber::yield() {
  ++yield_count_;
  ++sched_.total_yields_;
  // In this toy, yield just returns; the scheduler will requeue us.
}

inline Fiber* Scheduler::spawn(Fiber::Step step) {
  auto* f = new Fiber(*this, std::move(step));
  ready_.push(f);
  return f;
}

inline void Scheduler::requeue(Fiber* f) {
  if (!f->done_) ready_.push(f);
}

inline void Scheduler::run() {
  while (!ready_.empty()) {
    Fiber* f = ready_.front();
    ready_.pop();
    bool more = f->step_(*f);
    if (!more) {
      f->done_ = true;
      delete f;
    } else {
      requeue(f);
    }
  }
}
