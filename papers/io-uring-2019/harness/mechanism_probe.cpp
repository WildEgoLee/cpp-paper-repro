// io_uring mechanism probe: user submits SQEs, simulated kernel posts CQEs.

#include "../model/minimal/ring.hpp"
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
  constexpr size_t N = 8;          // power of two
  SubmissionQueue sq(N);
  CompletionQueue cq(N);
  std::atomic<bool> stop{false};

  // Simulated kernel thread: consume SQ, produce CQ
  std::thread kernel([&] {
    while (!stop.load(std::memory_order_acquire)) {
      if (auto sqe = sq.try_pop()) {
        CQE cqe;
        cqe.user_data = sqe->user_data;
        cqe.res = static_cast<int32_t>(sqe->len);  // echo length as "bytes"
        while (!cq.try_push(cqe)) {
          std::this_thread::yield();
        }
      } else {
        std::this_thread::yield();
      }
    }
  });

  // User: submit several SQEs
  const int kOps = 5;
  for (int i = 0; i < kOps; ++i) {
    SQE sqe;
    sqe.opcode = 1;
    sqe.user_data = static_cast<uint64_t>(i);
    sqe.len = 100 + i;
    while (!sq.try_push(sqe)) std::this_thread::yield();
  }

  // Collect completions
  std::vector<CQE> got;
  while (got.size() < static_cast<size_t>(kOps)) {
    if (auto cqe = cq.try_pop()) got.push_back(*cqe);
    else std::this_thread::yield();
  }

  stop.store(true, std::memory_order_release);
  kernel.join();

  printf("{\n");
  printf("  \"probe\": \"io-uring-mechanism\",\n");
  printf("  \"submitted\": %d,\n", kOps);
  printf("  \"completed\": %zu,\n", got.size());
  printf("  \"user_data_ok\": %s,\n", (got.size()==kOps && got[0].user_data==0 && got[4].user_data==4) ? "true" : "false");
  printf("  \"res_ok\": %s,\n", (got[0].res==100 && got[4].res==104) ? "true" : "false");
  printf("  \"note\": \"user-space simulated SQ/CQ rings; no real kernel\"\n");
  printf("}\n");

  return (got.size() == kOps) ? 0 : 1;
}
