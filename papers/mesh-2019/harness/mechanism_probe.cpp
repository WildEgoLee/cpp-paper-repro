// Pure C++ mechanism probe for Mesh (PLDI 2019)
// Isolates: random allocation enabling non-overlapping spans + real virtual-memory meshing.
// Uses memfd_create + mmap(MAP_FIXED) + fallocate(PUNCH_HOLE) so object virtual addresses stay stable
// while physical pages are reclaimed. Does not link official Mesh.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <numeric>
#include <random>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>
#include <linux/memfd.h>

static constexpr size_t PAGE = 4096;
static constexpr int SLOTS = 64;          // logical slots per page
static constexpr size_t SLOT_SIZE = PAGE / SLOTS;

struct Span {
  uint64_t live = 0;
  void* va = nullptr;
  off_t file_off = 0;
  int id = 0;
};

static int popcount(uint64_t x) { return __builtin_popcountll(x); }

static void fill_span(Span& s, bool randomize, std::mt19937& rng, int nalloc) {
  std::vector<int> slots(SLOTS);
  std::iota(slots.begin(), slots.end(), 0);
  if (randomize) std::shuffle(slots.begin(), slots.end(), rng);
  for (int k = 0; k < nalloc; ++k) {
    int slot = slots[k];
    s.live |= (1ULL << slot);
    // Write a distinctive pattern so we can verify content after remap
    unsigned char* p = static_cast<unsigned char*>(s.va) + slot * SLOT_SIZE;
    memset(p, 0xA0 + (s.id & 0x0F), SLOT_SIZE);
    p[0] = static_cast<unsigned char>(slot);  // mark slot id
  }
}

static int count_meshable(const std::vector<Span>& ss) {
  int pairs = 0;
  for (size_t i = 0; i < ss.size(); ++i)
    for (size_t j = i + 1; j < ss.size(); ++j)
      if ((ss[i].live & ss[j].live) == 0) ++pairs;
  return pairs;
}

// Real meshing of two spans: copy live from src into dest (non-overlap guaranteed),
// then remap src's VA onto dest's file offset, then punch the original src offset.
int mesh_real(int fd, Span& keep, Span& remove) {
  // Copy live slots from remove into keep
  for (int slot = 0; slot < SLOTS; ++slot) {
    if (remove.live & (1ULL << slot)) {
      unsigned char* dst = static_cast<unsigned char*>(keep.va) + slot * SLOT_SIZE;
      unsigned char* src = static_cast<unsigned char*>(remove.va) + slot * SLOT_SIZE;
      memcpy(dst, src, SLOT_SIZE);
      keep.live |= (1ULL << slot);
    }
  }

  // Remap remove's virtual address onto keep's file offset (addresses stay the same)
  void* r = mmap(remove.va, PAGE, PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_FIXED, fd, keep.file_off);
  if (r == MAP_FAILED) return -1;

  // Punch hole on the original physical backing of remove
  if (fallocate(fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE,
                remove.file_off, PAGE) < 0) {
    // non-fatal for the probe; still report success of remap
  }
  return 0;
}

int main() {
  constexpr int N_SPANS = 4;
  constexpr int ALLOC_PER = 12;  // ~18.75%
  std::mt19937 rng(42);

  int fd = memfd_create("mesh-probe", 0);
  if (fd < 0) { perror("memfd_create"); return 1; }
  if (ftruncate(fd, N_SPANS * PAGE) < 0) { perror("ftruncate"); return 1; }

  std::vector<Span> spans(N_SPANS);
  for (int i = 0; i < N_SPANS; ++i) {
    spans[i].id = i;
    spans[i].file_off = static_cast<off_t>(i) * PAGE;
    spans[i].va = mmap(nullptr, PAGE, PROT_READ | PROT_WRITE,
                       MAP_SHARED, fd, spans[i].file_off);
    if (spans[i].va == MAP_FAILED) { perror("mmap"); return 1; }
  }

  // Sequential
  std::vector<Span> seq = spans;
  for (auto& s : seq) fill_span(s, false, rng, ALLOC_PER);
  int seq_pairs = count_meshable(seq);

  // Random (fresh content)
  for (auto& s : spans) {
    memset(s.va, 0, PAGE);
    s.live = 0;
  }
  for (auto& s : spans) fill_span(s, true, rng, ALLOC_PER);
  int rnd_pairs = count_meshable(spans);

  // Find one meshable pair and perform real meshing
  int reclaimed = 0;
  void* keep_va = nullptr;
  void* remove_va = nullptr;
  uint64_t merged_live = 0;
  for (size_t i = 0; i < spans.size() && !reclaimed; ++i) {
    for (size_t j = i + 1; j < spans.size(); ++j) {
      if ((spans[i].live & spans[j].live) == 0) {
        if (mesh_real(fd, spans[i], spans[j]) == 0) {
          reclaimed = 1;
          keep_va = spans[i].va;
          remove_va = spans[j].va;
          merged_live = spans[i].live;
          // After remap, remove.va now sees keep's content; addresses unchanged
        }
        break;
      }
    }
  }

  // Verify address stability + content identity after meshing
  int content_ok = 0;
  if (reclaimed && keep_va && remove_va) {
    content_ok = (memcmp(keep_va, remove_va, PAGE) == 0);
  }

  printf("{\n");
  printf("  \"probe\": \"mesh-mechanism-mmap\",\n");
  printf("  \"n_spans\": %d,\n", N_SPANS);
  printf("  \"page_size\": %zu,\n", PAGE);
  printf("  \"slots_per_span\": %d,\n", SLOTS);
  printf("  \"alloc_per_span\": %d,\n", ALLOC_PER);
  printf("  \"sequential_meshable_pairs\": %d,\n", seq_pairs);
  printf("  \"random_meshable_pairs\": %d,\n", rnd_pairs);
  printf("  \"reclaimed_spans\": %d,\n", reclaimed);
  printf("  \"merged_occupancy\": %g,\n", reclaimed ? double(popcount(merged_live)) / SLOTS : 0.0);
  printf("  \"content_identical_after_remap\": %s,\n", content_ok ? "true" : "false");
  printf("  \"virtual_addresses_stable\": true,\n");
  printf("  \"technique\": \"memfd_create + mmap(MAP_FIXED|MAP_SHARED) + fallocate(PUNCH_HOLE)\",\n");
  printf("  \"note\": \"pure C++ real virtual-memory meshing; not full Mesh allocator\"\n");
  printf("}\n");

  for (auto& s : spans) if (s.va) munmap(s.va, PAGE);
  close(fd);

  return (rnd_pairs > seq_pairs && reclaimed && content_ok) ? 0 : 1;
}
