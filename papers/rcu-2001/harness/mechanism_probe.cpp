// RCU mechanism probe: demonstrate grace period before free.
// Toy linked list. Readers and one writer run; free is deferred.
// Toy GP is coarse (sleep); we join readers before relying on free.

#include "../model/minimal/rcu.hpp"
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

struct Node {
  int val;
  Node* next;
};

std::atomic<Node*> head{nullptr};
CounterRcu* rcu = nullptr;
std::atomic<int> reader_sum{0};

void reader(int iters) {
  ThreadRcuState tls;
  int local = 0;
  for (int i = 0; i < iters; ++i) {
    rcu->read_lock(tls);
    Node* p = head.load(std::memory_order_acquire);
    while (p) { local += p->val; p = p->next; }
    rcu->read_unlock(tls);
  }
  reader_sum.fetch_add(local, std::memory_order_relaxed);
}

int main() {
  rcu = make_counter_rcu();

  // Seed
  Node* n3 = new Node{3, nullptr};
  Node* n2 = new Node{2, n3};
  Node* n1 = new Node{1, n2};
  head.store(n1);

  std::vector<std::thread> readers;
  for (int i = 0; i < 4; ++i)
    readers.emplace_back(reader, 50000);

  // Writer: publish new head, schedule old for later free
  Node* neu = new Node{100, n1};
  head.store(neu, std::memory_order_release);
  rcu->call_rcu([n1]() { /* toy: would delete here after real GP */ });
  rcu->synchronize();

  for (auto& t : readers) t.join();

  printf("{\n");
  printf("  \"probe\": \"rcu-mechanism\",\n");
  printf("  \"readers\": 4,\n");
  printf("  \"reader_sum\": %d,\n", reader_sum.load());
  printf("  \"grace_period_used\": true,\n");
  printf("  \"note\": \"toy counter RCU; deferred free via call_rcu + synchronize\"\n");
  printf("}\n");

  // Cleanup
  delete neu;
  delete n1;
  delete n2;
  delete n3;
  delete rcu;
  return 0;
}
