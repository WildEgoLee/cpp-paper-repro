// Work-stealing mechanism probe.
// Each worker has a deque; empty workers steal from a random victim's top.

#include "../model/minimal/deque.hpp"
#include <atomic>
#include <cstdio>
#include <random>
#include <thread>
#include <vector>

int main() {
  constexpr int kWorkers = 4;
  constexpr int kTasksPerWorker = 1000;

  std::vector<WorkStealingDeque<int>> deques(kWorkers);
  std::atomic<int> completed{0};
  std::atomic<int> stolen{0};

  std::vector<std::thread> workers;
  for (int w = 0; w < kWorkers; ++w) {
    workers.emplace_back([&, w] {
      std::mt19937 rng(w + 1);
      // Seed local work
      for (int i = 0; i < kTasksPerWorker; ++i)
        deques[w].push_bottom(w * kTasksPerWorker + i);

      while (completed.load(std::memory_order_relaxed) < kWorkers * kTasksPerWorker) {
        if (auto task = deques[w].pop_bottom()) {
          completed.fetch_add(1, std::memory_order_relaxed);
        } else {
          int victim = static_cast<int>(rng() % kWorkers);
          if (victim != w) {
            if (auto task = deques[victim].steal_top()) {
              stolen.fetch_add(1, std::memory_order_relaxed);
              completed.fetch_add(1, std::memory_order_relaxed);
            }
          }
        }
      }
    });
  }

  for (auto& t : workers) t.join();

  int total = kWorkers * kTasksPerWorker;
  std::printf("{\n");
  std::printf("  \"probe\": \"work-stealing\",\n");
  std::printf("  \"workers\": %d,\n", kWorkers);
  std::printf("  \"tasks\": %d,\n", total);
  std::printf("  \"completed\": %d,\n", completed.load());
  std::printf("  \"stolen\": %d,\n", stolen.load());
  std::printf("  \"all_done\": %s,\n", completed.load() == total ? "true" : "false");
  std::printf("  \"note\": \"toy work-stealing; mutex deque, random victim\"\n");
  std::printf("}\n");

  return completed.load() == total ? 0 : 1;
}
