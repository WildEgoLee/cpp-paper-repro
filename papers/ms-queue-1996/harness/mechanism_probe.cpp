// Michael-Scott queue mechanism probe.
// Tagged pointers + hazard pointer reclamation.

#include "../model/minimal/ms_queue.hpp"
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
  constexpr int kProducers = 4;
  constexpr int kConsumers = 4;
  constexpr int kPerProducer = 10000;

  MSQueue<int> q;
  std::atomic<int> produced{0};
  std::atomic<int> consumed{0};
  std::atomic<long long> sum{0};

  std::vector<std::thread> producers;
  for (int p = 0; p < kProducers; ++p) {
    producers.emplace_back([&, p] {
      for (int i = 0; i < kPerProducer; ++i) {
        int v = p * kPerProducer + i;
        q.enqueue(v);
        produced.fetch_add(1, std::memory_order_relaxed);
      }
    });
  }

  std::vector<std::thread> consumers;
  for (int c = 0; c < kConsumers; ++c) {
    consumers.emplace_back([&] {
      while (consumed.load(std::memory_order_relaxed) < kProducers * kPerProducer) {
        if (auto v = q.dequeue()) {
          sum.fetch_add(*v, std::memory_order_relaxed);
          consumed.fetch_add(1, std::memory_order_relaxed);
        } else {
          std::this_thread::yield();
        }
      }
    });
  }

  for (auto& t : producers) t.join();
  for (auto& t : consumers) t.join();

  long long N = static_cast<long long>(kProducers) * kPerProducer;
  long long expected = (N - 1) * N / 2;

  std::printf("{\n");
  std::printf("  \"probe\": \"ms-queue\",\n");
  std::printf("  \"producers\": %d,\n", kProducers);
  std::printf("  \"consumers\": %d,\n", kConsumers);
  std::printf("  \"produced\": %d,\n", produced.load());
  std::printf("  \"consumed\": %d,\n", consumed.load());
  std::printf("  \"sum\": %lld,\n", sum.load());
  std::printf("  \"expected_sum\": %lld,\n", expected);
  std::printf("  \"correct\": %s,\n", (consumed.load() == N && sum.load() == expected) ? "true" : "false");
  std::printf("  \"aba_protection\": \"tagged-pointer\",\n");
  std::printf("  \"reclamation\": \"hazard-pointer\",\n");
  std::printf("  \"note\": \"MS queue with tagged pointers + hazard pointer safe free\"\n");
  std::printf("}\n");

  return (consumed.load() == N && sum.load() == expected) ? 0 : 1;
}
