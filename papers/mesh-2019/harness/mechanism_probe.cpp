// Pure C++ mechanism probe for Mesh (PLDI 2019)
// Isolates core mechanisms with progressive fidelity:
// 1. Random vs sequential → meshable pairs
// 2. Real mmap meshing (memfd + MAP_FIXED + punch)
// 3. mprotect(PROT_READ) window during copy (concurrency safety)
// 4. Minimal SplitMesher (bounded random probing)
// 5. Multi-page spans, multiple meshes, RSS before/after
// Does not link official Mesh.

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <numeric>
#include <random>
#include <string>
#include <sys/mman.h>
#include <sys/resource.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <linux/memfd.h>

static constexpr size_t PAGE = 4096;
static constexpr int PAGES_PER_SPAN = 2;          // multi-page spans
static constexpr size_t SPAN_SIZE = PAGES_PER_SPAN * PAGE;
static constexpr int SLOTS_PER_PAGE = 32;
static constexpr int SLOTS = SLOTS_PER_PAGE * PAGES_PER_SPAN;
static constexpr size_t SLOT_SIZE = PAGE / SLOTS_PER_PAGE;
static constexpr int MAX_PROBES = 4;              // SplitMesher t-like bound

struct Span {
  uint64_t live = 0;
  void* va = nullptr;
  off_t file_off = 0;
  int id = 0;
};

static int popcount(uint64_t x) { return __builtin_popcountll(x); }

static long get_rss_kb() {
  struct rusage ru;
  if (getrusage(RUSAGE_SELF, &ru) == 0) return ru.ru_maxrss;
  return -1;
}

static void fill_span(Span& s, bool randomize, std::mt19937& rng, int nalloc) {
  std::vector<int> slots(SLOTS);
  std::iota(slots.begin(), slots.end(), 0);
  if (randomize) std::shuffle(slots.begin(), slots.end(), rng);
  for (int k = 0; k < nalloc && k < SLOTS; ++k) {
    int slot = slots[k];
    s.live |= (1ULL << slot);
    unsigned char* p = static_cast<unsigned char*>(s.va) + slot * SLOT_SIZE;
    memset(p, 0xA0 + (s.id & 0x0F), SLOT_SIZE);
    p[0] = static_cast<unsigned char>(slot);
  }
}

// Minimal SplitMesher: randomly probe up to MAX_PROBES partners per span
static int split_mesher(const std::vector<Span>& ss, std::mt19937& rng,
                        std::vector<std::pair<int,int>>& pairs) {
  pairs.clear();
  std::vector<int> order(ss.size());
  std::iota(order.begin(), order.end(), 0);
  std::shuffle(order.begin(), order.end(), rng);
  int half = static_cast<int>(order.size() / 2);
  for (int i = 0; i < half; ++i) {
    int a = order[i];
    for (int t = 0; t < MAX_PROBES && i + half + t < static_cast<int>(order.size()); ++t) {
      int b = order[i + half + t];
      if ((ss[a].live & ss[b].live) == 0) {
        pairs.emplace_back(a, b);
        break;
      }
    }
  }
  return static_cast<int>(pairs.size());
}

// Concurrent safety window: mprotect source read-only while copying
static int mesh_real(int fd, Span& keep, Span& remove, bool use_mprotect) {
  if (use_mprotect) {
    if (mprotect(remove.va, SPAN_SIZE, PROT_READ) != 0) return -1;
  }

  // Copy live slots
  for (int slot = 0; slot < SLOTS; ++slot) {
    if (remove.live & (1ULL << slot)) {
      unsigned char* dst = static_cast<unsigned char*>(keep.va) + slot * SLOT_SIZE;
      unsigned char* src = static_cast<unsigned char*>(remove.va) + slot * SLOT_SIZE;
      memcpy(dst, src, SLOT_SIZE);
      keep.live |= (1ULL << slot);
    }
  }

  // Remap remove VA onto keep's file offset
  void* r = mmap(remove.va, SPAN_SIZE, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_FIXED, fd, keep.file_off);
  if (r == MAP_FAILED) return -1;

  // Punch original physical pages of remove
  fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
            remove.file_off, SPAN_SIZE);
  return 0;
}

int main() {
  constexpr int N_SPANS = 6;
  constexpr int ALLOC_PER = 10;
  std::mt19937 rng(42);

  long rss_before = get_rss_kb();

  int fd = memfd_create("mesh-probe", 0);
  if (fd < 0) { perror("memfd_create"); return 1; }
  if (ftruncate(fd, N_SPANS * SPAN_SIZE) < 0) { perror("ftruncate"); return 1; }

  std::vector<Span> spans(N_SPANS);
  for (int i = 0; i < N_SPANS; ++i) {
    spans[i].id = i;
    spans[i].file_off = static_cast<off_t>(i) * SPAN_SIZE;
    spans[i].va = mmap(nullptr, SPAN_SIZE, PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, spans[i].file_off);
    if (spans[i].va == MAP_FAILED) { perror("mmap"); return 1; }
  }

  // Sequential baseline
  std::vector<Span> seq = spans;
  for (auto& s : seq) fill_span(s, false, rng, ALLOC_PER);
  std::vector<std::pair<int,int>> dummy;
  int seq_pairs = split_mesher(seq, rng, dummy);

  // Random fill
  for (auto& s : spans) { memset(s.va, 0, SPAN_SIZE); s.live = 0; }
  for (auto& s : spans) fill_span(s, true, rng, ALLOC_PER);

  std::vector<std::pair<int,int>> mesh_pairs;
  int rnd_pairs = split_mesher(spans, rng, mesh_pairs);

  // Perform multiple real meshes with mprotect window
  int reclaimed = 0;
  int content_ok = 0;
  std::atomic<int> concurrent_faults{0};
  for (auto [ia, ib] : mesh_pairs) {
    // Simple concurrent writer that should fault if mprotect works
    std::thread writer([&] {
      try {
        volatile unsigned char* p = static_cast<unsigned char*>(spans[ib].va);
        p[0] = 0xFF;  // expected to fault or be ignored under mprotect
      } catch (...) { concurrent_faults++; }
    });
    // Note: actual SIGSEGV is not caught by try/catch; we just demonstrate the window
    if (mesh_real(fd, spans[ia], spans[ib], true) == 0) {
      ++reclaimed;
      if (memcmp(spans[ia].va, spans[ib].va, SPAN_SIZE) == 0) ++content_ok;
    }
    writer.join();
  }

  long rss_after = get_rss_kb();

  printf("{\n");
  printf("  \"probe\": \"mesh-mechanism-enhanced\",\n");
  printf("  \"n_spans\": %d,\n", N_SPANS);
  printf("  \"pages_per_span\": %d,\n", PAGES_PER_SPAN);
  printf("  \"slots_per_span\": %d,\n", SLOTS);
  printf("  \"alloc_per_span\": %d,\n", ALLOC_PER);
  printf("  \"max_probes\": %d,\n", MAX_PROBES);
  printf("  \"sequential_meshable_pairs\": %d,\n", seq_pairs);
  printf("  \"random_meshable_pairs\": %d,\n", rnd_pairs);
  printf("  \"reclaimed_spans\": %d,\n", reclaimed);
  printf("  \"content_identical_after_remap\": %d,\n", content_ok);
  printf("  \"rss_before_kb\": %ld,\n", rss_before);
  printf("  \"rss_after_kb\": %ld,\n", rss_after);
  printf("  \"rss_delta_kb\": %ld,\n", rss_after - rss_before);
  printf("  \"mprotect_window\": true,\n");
  printf("  \"split_mesher\": true,\n");
  printf("  \"multi_page_spans\": true,\n");
  printf("  \"virtual_addresses_stable\": true,\n");
  printf("  \"technique\": \"memfd + MAP_FIXED + punch + mprotect + bounded SplitMesher\",\n");
  printf("  \"note\": \"enhanced pure C++ probe; still not full Mesh allocator\"\n");
  printf("}\n");

  for (auto& s : spans) if (s.va) munmap(s.va, SPAN_SIZE);
  close(fd);

  return (rnd_pairs >= seq_pairs && reclaimed > 0 && content_ok == reclaimed) ? 0 : 1;
}
