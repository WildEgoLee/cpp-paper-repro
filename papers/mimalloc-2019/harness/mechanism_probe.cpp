// Mechanism probe for Mimalloc: Free List Sharding in Action (APLAS 2019).
//
// This is NOT a reproduction of the paper's figures. It isolates four design
// claims on a single machine so a later full-bench run can be compared against
// a mechanism, not just a ranking:
//   1. page-local allocation locality vs a strided monolithic free list
//   2. free-list pop vs bump-pointer fast path
//   3. temporal cadence: maintenance on the empty-list slow path
//   4. cross-thread free: one global lock vs a page-local atomic thread_free
//   5. allocator-induced false sharing: same cache line vs per-line objects
//
// Build: make -C papers/mimalloc-2019/harness
// Run:   ./mechanism_probe            # JSON on stdout
//        ./mechanism_probe --self-test

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

constexpr int kReps = 7;

struct alignas(64) Node {
  Node* next;
  Node* link;
  uint64_t stamp;
  unsigned char pad[64 - sizeof(Node*) - sizeof(Node*) - sizeof(uint64_t)];
};
static_assert(sizeof(Node) == 64, "node must occupy one cache line");

double median(std::vector<double> v) {
  std::sort(v.begin(), v.end());
  const size_t n = v.size();
  if (n == 0) return 0;
  if (n % 2) return v[n / 2];
  return 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

double now_ns() {
  using clock = std::chrono::steady_clock;
  return std::chrono::duration<double, std::nano>(clock::now().time_since_epoch()).count();
}

uint64_t rdtsc_serial() {
  unsigned lo, hi;
  asm volatile("lfence\n\trdtsc" : "=a"(lo), "=d"(hi)::"memory");
  return (static_cast<uint64_t>(hi) << 32) | lo;
}

void pin_json_string(std::string& out, const char* s) {
  out.push_back('"');
  for (const char* p = s; *p; ++p) {
    if (*p == '"' || *p == '\\') out.push_back('\\');
    out.push_back(*p);
  }
  out.push_back('"');
}

// ---------------------------------------------------------------------------
// 1. Locality. Temporally adjacent allocations are either sequential inside
//    64 KiB-sized runs, or taken with a stride that lands on a different page.
//    The timed section is the later pointer-chase, not the allocation itself.
// ---------------------------------------------------------------------------

uint64_t chase(Node* head, int passes) {
  uint64_t acc = 0;
  for (int p = 0; p < passes; ++p) {
    for (Node* n = head; n != nullptr; n = n->link) acc += n->stamp;
  }
  return acc;
}

double locality_ns_per_visit(bool sharded, uint64_t* checksum) {
  constexpr size_t N = 1u << 20;  // 64 MiB
  constexpr int kPasses = 4;
  std::vector<Node> nodes(N);
  for (size_t i = 0; i < N; ++i) nodes[i].stamp = (i * 17u) + 3u;

  if (sharded) {
    for (size_t i = 0; i + 1 < N; ++i) nodes[i].link = &nodes[i + 1];
    nodes[N - 1].link = nullptr;
  } else {
    // 1025 is coprime with 2^20, and 1025*64 B is just over a 64 KiB page,
    // so successive allocations do not share a software page.
    constexpr size_t stride = 1025;
    for (size_t i = 0; i < N; ++i) {
      const size_t cur = (i * stride) % N;
      const size_t nxt = ((i + 1) * stride) % N;
      nodes[cur].link = (i + 1 < N) ? &nodes[nxt] : nullptr;
    }
  }

  chase(&nodes[0], 1);  // warm TLB
  const double t0 = now_ns();
  const uint64_t acc = chase(&nodes[0], kPasses);
  const double t1 = now_ns();
  *checksum ^= acc;
  return (t1 - t0) / static_cast<double>(N * kPasses);
}

// ---------------------------------------------------------------------------
// 2. Fast path. One self-linked free-list pop/push versus a bump pointer that
//    wraps a 64 KiB page. Checksums keep the loops live under -O2.
// ---------------------------------------------------------------------------

// noipa: this TU must not delete the dependent load just because some caller
// builds a tiny circular list. There is no per-iteration asm barrier.
__attribute__((noinline, noipa)) uint64_t freelist_pop(Node* head, uint64_t iters) {
  uint64_t acc = 0;
  for (uint64_t i = 0; i < iters; ++i) {
    Node* b = head;
    head = b->next;
    acc += b->stamp;
  }
  return acc + reinterpret_cast<uintptr_t>(head);
}

__attribute__((noinline, noipa)) uint64_t bump_wrap(unsigned char* page, size_t bytes, uint64_t iters) {
  unsigned char* bump = page;
  unsigned char* end = page + bytes;
  uint64_t acc = 0;
  for (uint64_t i = 0; i < iters; ++i) {
    if (bump + 64 > end) bump = page;
    unsigned char* p = bump;
    bump += 64;
    acc += p[0];
    p[0] = static_cast<unsigned char>(acc);
  }
  return acc + reinterpret_cast<uintptr_t>(bump);
}

struct FastSample {
  double ns_per_op;
  double cycles_per_op;
};

FastSample time_fast(bool bump, uint64_t* checksum) {
  constexpr uint64_t kIters = 20000000ull;
  Node ring[128];
  for (int i = 0; i < 128; ++i) {
    ring[i].next = &ring[(i + 1) % 128];
    ring[i].stamp = static_cast<uint64_t>(i) * 2u + 1u;
  }
  alignas(64) unsigned char page[64 * 1024];
  std::memset(page, 1, sizeof(page));

  if (bump) bump_wrap(page, sizeof(page), 1024);
  else freelist_pop(ring, 1024);

  const uint64_t c0 = rdtsc_serial();
  const double t0 = now_ns();
  const uint64_t acc = bump ? bump_wrap(page, sizeof(page), kIters) : freelist_pop(ring, kIters);
  const double t1 = now_ns();
  const uint64_t c1 = rdtsc_serial();
  *checksum ^= acc;
  FastSample s;
  s.ns_per_op = (t1 - t0) / static_cast<double>(kIters);
  s.cycles_per_op = static_cast<double>(c1 - c0) / static_cast<double>(kIters);
  return s;
}

// ---------------------------------------------------------------------------
// 3. Temporal cadence. Both variants perform the same maintenance loads.
//    Eager pays them on every pop. Cadence pays them only when the page-local
//    free list runs dry (the natural slow path — no extra counter on the pop).
// ---------------------------------------------------------------------------

constexpr int kPageBlocks = 512;
std::atomic<uint64_t> g_flags[4];

uint64_t maintenance(uint64_t state) {
  // memory clobber: relaxed atomic loads must not be hoisted out of the loop,
  // or the eager path silently stops paying for its checks.
  for (int i = 0; i < 4; ++i) {
    const uint64_t f = g_flags[i].load(std::memory_order_relaxed);
    state += f;
  }
  asm volatile("" : "+r"(state)::"memory");
  return state;
}

void refill(Node* storage, Node*& free_list) {
  for (int i = 0; i < kPageBlocks - 1; ++i) storage[i].next = &storage[i + 1];
  storage[kPageBlocks - 1].next = nullptr;
  free_list = &storage[0];
}

__attribute__((noinline, noipa)) uint64_t cadence_loop(Node* storage, uint64_t iters) {
  Node* free_list = nullptr;
  refill(storage, free_list);
  uint64_t state = 1;
  for (uint64_t i = 0; i < iters; ++i) {
    if (free_list == nullptr) {
      for (int j = 0; j < kPageBlocks; ++j) state = maintenance(state);
      refill(storage, free_list);
    }
    Node* b = free_list;
    free_list = b->next;
    b->stamp = state;
  }
  return state;
}

__attribute__((noinline, noipa)) uint64_t eager_loop(Node* storage, uint64_t iters) {
  Node* free_list = nullptr;
  refill(storage, free_list);
  uint64_t state = 1;
  for (uint64_t i = 0; i < iters; ++i) {
    if (free_list == nullptr) refill(storage, free_list);
    Node* b = free_list;
    free_list = b->next;
    b->stamp = state;
    state = maintenance(state);
  }
  return state;
}

double time_cadence(bool eager, uint64_t* checksum) {
  constexpr uint64_t kIters = 4000000ull;
  Node storage[kPageBlocks];
  for (int i = 0; i < 4; ++i) g_flags[i].store(0, std::memory_order_relaxed);
  if (eager) eager_loop(storage, 64);
  else cadence_loop(storage, 64);
  const double t0 = now_ns();
  const uint64_t acc = eager ? eager_loop(storage, kIters) : cadence_loop(storage, kIters);
  const double t1 = now_ns();
  *checksum ^= acc;
  return (t1 - t0) / static_cast<double>(kIters);
}

// ---------------------------------------------------------------------------
// 4. Cross-thread free. Producer thread owns the page. The peer frees onto
//    either one process-wide mutex or that page's atomic thread_free stack.
// ---------------------------------------------------------------------------

uint64_t pack_ptr(Node* p, uint16_t tag) {
  return (static_cast<uint64_t>(tag) << 48) | (reinterpret_cast<uintptr_t>(p) & 0x0000FFFFFFFFFFFFull);
}
Node* unpack_ptr(uint64_t t) { return reinterpret_cast<Node*>(t & 0x0000FFFFFFFFFFFFull); }
uint16_t unpack_tag(uint64_t t) { return static_cast<uint16_t>(t >> 48); }

struct Heap {
  Node* free_list = nullptr;
  std::atomic<uint64_t> thread_free{0};
};

struct GlobalPool {
  std::mutex mu;
  Node* free_list = nullptr;
};

void remote_free(Heap& heap, Node* n) {
  uint64_t head = heap.thread_free.load(std::memory_order_relaxed);
  for (;;) {
    n->next = unpack_ptr(head);
    const uint64_t next = pack_ptr(n, static_cast<uint16_t>(unpack_tag(head) + 1));
    if (heap.thread_free.compare_exchange_weak(head, next, std::memory_order_release,
                                                std::memory_order_relaxed)) {
      return;
    }
  }
}

void drain_thread_free(Heap& heap) {
  const uint64_t old = heap.thread_free.exchange(0, std::memory_order_acquire);
  Node* p = unpack_ptr(old);
  while (p != nullptr) {
    Node* nxt = p->next;
    p->next = heap.free_list;
    heap.free_list = p;
    p = nxt;
  }
}

Node* sharded_alloc(Heap& heap) {
  if (heap.free_list == nullptr) drain_thread_free(heap);
  Node* b = heap.free_list;
  if (b == nullptr) return nullptr;
  heap.free_list = b->next;
  return b;
}

size_t count_list(Node* p) {
  size_t n = 0;
  while (p != nullptr) {
    p = p->next;
    ++n;
  }
  return n;
}

struct Barrier {
  explicit Barrier(int n) : nthreads(n) {}
  void arrive() {
    const int gen = generation.load(std::memory_order_acquire);
    if (count.fetch_add(1, std::memory_order_acq_rel) + 1 == nthreads) {
      count.store(0, std::memory_order_relaxed);
      generation.fetch_add(1, std::memory_order_release);
    } else {
      while (generation.load(std::memory_order_acquire) == gen) std::this_thread::yield();
    }
  }
  int nthreads;
  std::atomic<int> count{0};
  std::atomic<int> generation{0};
};

constexpr int kBatch = 2048;
constexpr int kRounds = 200;

double cross_thread_sharded(uint64_t* checksum, bool check_conserve) {
  std::vector<Node> arena(static_cast<size_t>(kBatch) * 2);
  Heap heaps[2];
  for (int t = 0; t < 2; ++t) {
    Node* head = nullptr;
    for (int i = 0; i < kBatch; ++i) {
      Node* n = &arena[static_cast<size_t>(t * kBatch + i)];
      if ((reinterpret_cast<uintptr_t>(n) >> 48) != 0) {
        std::fprintf(stderr, "tagged-pointer assumption failed\n");
        std::exit(2);
      }
      n->next = head;
      head = n;
    }
    heaps[t].free_list = head;
  }

  Node* slots[2][kBatch];
  Barrier bar(2);
  std::atomic<int> failed{0};

  auto worker = [&](int me) {
    for (int r = 0; r < kRounds; ++r) {
      for (int i = 0; i < kBatch; ++i) {
        Node* n = sharded_alloc(heaps[me]);
        if (n == nullptr) {
          failed.store(1, std::memory_order_relaxed);
          return;
        }
        slots[me][i] = n;
      }
      bar.arrive();
      for (int i = 0; i < kBatch; ++i) remote_free(heaps[1 - me], slots[1 - me][i]);
      bar.arrive();
    }
  };

  const double t0 = now_ns();
  std::thread a(worker, 0);
  std::thread b(worker, 1);
  a.join();
  b.join();
  const double t1 = now_ns();
  if (failed.load()) {
    std::fprintf(stderr, "sharded alloc ran dry\n");
    std::exit(2);
  }
  if (check_conserve) {
    for (int t = 0; t < 2; ++t) {
      drain_thread_free(heaps[t]);
      const size_t got = count_list(heaps[t].free_list);
      if (got != static_cast<size_t>(kBatch)) {
        std::fprintf(stderr, "sharded lost nodes on heap %d: %zu\n", t, got);
        std::exit(2);
      }
      *checksum += got;
    }
  }
  const double ops = static_cast<double>(kRounds) * kBatch * 2 * 2;  // alloc+free, both threads
  return (t1 - t0) / ops;
}

double cross_thread_global(uint64_t* checksum, bool check_conserve) {
  std::vector<Node> arena(static_cast<size_t>(kBatch) * 2);
  GlobalPool pool;
  for (size_t i = 0; i < arena.size(); ++i) {
    arena[i].next = pool.free_list;
    pool.free_list = &arena[i];
  }
  Node* slots[2][kBatch];
  Barrier bar(2);

  auto alloc = [&]() -> Node* {
    std::lock_guard<std::mutex> lock(pool.mu);
    Node* b = pool.free_list;
    if (b == nullptr) return nullptr;
    pool.free_list = b->next;
    return b;
  };
  auto free_n = [&](Node* n) {
    std::lock_guard<std::mutex> lock(pool.mu);
    n->next = pool.free_list;
    pool.free_list = n;
  };

  std::atomic<int> failed{0};
  auto worker = [&](int me) {
    for (int r = 0; r < kRounds; ++r) {
      for (int i = 0; i < kBatch; ++i) {
        Node* n = alloc();
        if (n == nullptr) {
          failed.store(1, std::memory_order_relaxed);
          return;
        }
        slots[me][i] = n;
      }
      bar.arrive();
      for (int i = 0; i < kBatch; ++i) free_n(slots[1 - me][i]);
      bar.arrive();
    }
  };

  const double t0 = now_ns();
  std::thread a(worker, 0);
  std::thread b(worker, 1);
  a.join();
  b.join();
  const double t1 = now_ns();
  if (failed.load()) {
    std::fprintf(stderr, "global alloc ran dry\n");
    std::exit(2);
  }
  if (check_conserve) {
    const size_t got = count_list(pool.free_list);
    if (got != arena.size()) {
      std::fprintf(stderr, "global lost nodes: %zu\n", got);
      std::exit(2);
    }
    *checksum += got;
  }
  const double ops = static_cast<double>(kRounds) * kBatch * 2 * 2;
  return (t1 - t0) / ops;
}

// ---------------------------------------------------------------------------
// 5. False sharing. Two threads hammer atomics that either share a line or sit
//    on dedicated lines. This is the cache-scratch shape, not a full allocator.
// ---------------------------------------------------------------------------

double false_share(bool separated) {
  constexpr uint64_t kIters = 8000000ull;
  struct Pack {
    std::atomic<uint64_t> a{0};
    std::atomic<uint64_t> b{0};
  };
  struct Sep {
    alignas(64) std::atomic<uint64_t> a{0};
    alignas(64) std::atomic<uint64_t> b{0};
  };
  Pack pack;
  Sep sep;
  std::atomic<uint64_t>* cells[2];
  if (separated) {
    cells[0] = &sep.a;
    cells[1] = &sep.b;
  } else {
    cells[0] = &pack.a;
    cells[1] = &pack.b;
  }
  std::atomic<int> go{0};
  double elapsed[2] = {0, 0};
  auto worker = [&](int me) {
    while (go.load(std::memory_order_acquire) == 0) std::this_thread::yield();
    const double t0 = now_ns();
    std::atomic<uint64_t>* cell = cells[me];
    for (uint64_t i = 0; i < kIters; ++i) cell->fetch_add(1, std::memory_order_relaxed);
    elapsed[me] = now_ns() - t0;
  };
  std::thread a(worker, 0);
  std::thread b(worker, 1);
  go.store(1, std::memory_order_release);
  a.join();
  b.join();
  return std::max(elapsed[0], elapsed[1]) / static_cast<double>(kIters);
}

void append_variant(std::string& out, const char* name, const std::vector<double>& samples, const char* unit,
                    bool comma) {
  if (comma) out += ",\n";
  out += "      {\"name\": ";
  pin_json_string(out, name);
  out += ", \"unit\": ";
  pin_json_string(out, unit);
  out += ", \"median\": ";
  char buf[64];
  std::snprintf(buf, sizeof(buf), "%.6f", median(samples));
  out += buf;
  out += ", \"samples\": [";
  for (size_t i = 0; i < samples.size(); ++i) {
    if (i) out += ", ";
    std::snprintf(buf, sizeof(buf), "%.6f", samples[i]);
    out += buf;
  }
  out += "]}";
}

}  // namespace

int main(int argc, char** argv) {
  const bool self_test = argc > 1 && std::strcmp(argv[1], "--self-test") == 0;
  const int reps = self_test ? 1 : kReps;
  uint64_t checksum = 0;

  if (self_test) {
    // Shrink by running the conserve checks once via the real functions.
    // Full functions use compile-time round counts; conservation is checked
    // on every call when the flag is set, including a short extra call below.
    uint64_t sink = 0;
    cross_thread_sharded(&sink, true);
    cross_thread_global(&sink, true);
    std::printf("self-test ok checksum=%llu\n", static_cast<unsigned long long>(sink));
    return 0;
  }

  std::vector<double> sharded_walk, strided_walk;
  std::vector<double> pop_ns, bump_ns, pop_cyc, bump_cyc;
  std::vector<double> eager_ns, cadence_ns;
  std::vector<double> global_ns, atomic_ns;
  std::vector<double> packed_ns, split_ns;

  for (int r = 0; r < reps; ++r) {
    sharded_walk.push_back(locality_ns_per_visit(true, &checksum));
    strided_walk.push_back(locality_ns_per_visit(false, &checksum));
    {
      const FastSample a = time_fast(false, &checksum);
      const FastSample b = time_fast(true, &checksum);
      pop_ns.push_back(a.ns_per_op);
      pop_cyc.push_back(a.cycles_per_op);
      bump_ns.push_back(b.ns_per_op);
      bump_cyc.push_back(b.cycles_per_op);
    }
    eager_ns.push_back(time_cadence(true, &checksum));
    cadence_ns.push_back(time_cadence(false, &checksum));
    global_ns.push_back(cross_thread_global(&checksum, false));
    atomic_ns.push_back(cross_thread_sharded(&checksum, false));
    packed_ns.push_back(false_share(false));
    split_ns.push_back(false_share(true));
  }

  // Conservation is part of the probe, not a timed sample.
  cross_thread_sharded(&checksum, true);
  cross_thread_global(&checksum, true);

  std::string out;
  out += "{\n";
  out += "  \"probe\": \"mimalloc-mechanism\",\n";
  out += "  \"disclaimer\": \"Not a reproduction of APLAS 2019 figures. Isolates free-list sharding, temporal cadence, thread_free, and false sharing on this host only.\",\n";
  out += "  \"paper\": \"Leijen, Zorn, de Moura. Mimalloc: Free List Sharding in Action. APLAS 2019.\",\n";
  out += "  \"historical_tag\": \"v1.0.0\",\n";
  char line[64];
  std::snprintf(line, sizeof(line), "  \"hardware_concurrency\": %u,\n", std::thread::hardware_concurrency());
  out += line;
  out += "  \"checksum\": ";
  std::snprintf(line, sizeof(line), "%llu", static_cast<unsigned long long>(checksum));
  out += line;
  out += ",\n  \"experiments\": [\n";

  out += "    {\n      \"id\": \"locality_walk\",\n";
  out += "      \"question\": \"Do temporally close allocations that stay inside a 64KiB page make the later pointer chase cheaper than a page-strided free list?\",\n";
  out += "      \"variants\": [\n";
  append_variant(out, "sharded_sequential_page", sharded_walk, "ns_per_visit", false);
  append_variant(out, "strided_across_pages", strided_walk, "ns_per_visit", true);
  out += "\n      ]\n    },\n";

  out += "    {\n      \"id\": \"fast_path\",\n";
  out += "      \"question\": \"Isolated L1-resident free-list pop versus a bump pointer that wraps a 64KiB page. Not the paper's in-allocator bump experiment.\",\n";
  out += "      \"variants\": [\n";
  append_variant(out, "freelist_pop_ns", pop_ns, "ns_per_op", false);
  append_variant(out, "bump_pointer_ns", bump_ns, "ns_per_op", true);
  append_variant(out, "freelist_pop_cycles", pop_cyc, "cycles_per_op", true);
  append_variant(out, "bump_pointer_cycles", bump_cyc, "cycles_per_op", true);
  out += "\n      ]\n    },\n";

  out += "    {\n      \"id\": \"temporal_cadence\",\n";
  out += "      \"question\": \"If the same maintenance runs either on every pop or only when the page free list is empty, which alloc path is faster?\",\n";
  out += "      \"variants\": [\n";
  append_variant(out, "eager_every_alloc", eager_ns, "ns_per_alloc", false);
  append_variant(out, "slow_path_when_empty", cadence_ns, "ns_per_alloc", true);
  out += "\n      ]\n    },\n";

  out += "    {\n      \"id\": \"cross_thread_free\",\n";
  out += "      \"question\": \"When each thread allocates locally and the peer frees, does a page-local atomic stack beat one global mutex?\",\n";
  out += "      \"variants\": [\n";
  append_variant(out, "global_mutex", global_ns, "ns_per_alloc_or_free", false);
  append_variant(out, "page_atomic_thread_free", atomic_ns, "ns_per_alloc_or_free", true);
  out += "\n      ]\n    },\n";

  out += "    {\n      \"id\": \"false_sharing\",\n";
  out += "      \"question\": \"Do two threads writing adjacent atomics lose to the same writes on separate cache lines?\",\n";
  out += "      \"variants\": [\n";
  append_variant(out, "same_cache_line", packed_ns, "ns_per_increment", false);
  append_variant(out, "split_cache_lines", split_ns, "ns_per_increment", true);
  out += "\n      ]\n    }\n";

  out += "  ]\n}\n";
  std::fwrite(out.data(), 1, out.size(), stdout);
  return 0;
}
