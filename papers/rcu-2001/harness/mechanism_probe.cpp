// RCU mechanism probe: demonstrate grace period before free.
// Toy linked list: writers publish new head, old nodes freed only after synchronize.

#include "../model/minimal/rcu.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>
#include <vector>

struct Node {
  int val;
  Node* next;
};

std::atomic<Node*> head{nullptr};
CounterRcu* rcu = nullptr;

void reader(int id, int iters) {
  ThreadRcuState tls;
  for (int i = 0; i < iters; ++i) {
    rcu->read_lock(tls);
    Node* p = head.load(std::memory_order_acquire);
    int sum = 0;
    while (p) { sum += p->val; p = p->next; }
    rcu->read_unlock(tls);
    // sink to prevent optimize-away
    if (sum == -1) std::printf(" ");
  }
}

void writer() {
  // Publish a new node at head
  Node* neu = new Node{100, head.load(std::memory_order_relaxed)};
  head.store(neu, std::memory_order_release);

  // Old head (if any) can be freed only after grace period
  Node* old = neu->next;
  if (old) {
    rcu->call_rcu([old]() { delete old; });
    rcu->synchronize();
  }
}

int main() {
  rcu = make_counter_rcu();

  // Seed list
  head.store(new Node{1, new Node{2, new Node{3, nullptr}}});

  std::vector<std::thread> readers;
  for (int i = 0; i < 4; ++i)
    readers.emplace_back(reader, i, 100000);

  std::thread w(writer);
  for (auto& t : readers) t.join();
  w.join();

  printf("{\n");
  printf("  \"probe\": \"rcu-mechanism\",\n");
  printf("  \"readers\": 4,\n");
  printf("  \"writer_updates\": 1,\n");
  printf("  \"grace_period_used\": true,\n");
  printf("  \"note\": \"toy counter RCU; old nodes freed only after synchronize\"\n");
  printf("}\n");

  // Cleanup remaining
  Node* p = head.load();
  while (p) { Node* n = p->next; delete p; p = n; }
  delete rcu;
  return 0;
}
