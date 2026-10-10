// Pure C++ mechanism probe for Mesh (PLDI 2019)
// Isolates: random allocation enabling non-overlapping spans, simulated meshing.
// Does not link official Mesh. Addresses are logical indices.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <string>
#include <vector>

struct Span {
  static constexpr int SLOTS = 64;  // objects per span (small for probe)
  uint64_t live = 0;                // bitset of live slots
  int id = 0;
};

static void print_json(const char* key, double val) {
  printf("  \"%s\": %g,\n", key, val);
}

static int popcount(uint64_t x) {
  return __builtin_popcountll(x);
}

int main() {
  constexpr int N_SPANS = 8;
  constexpr int ALLOC_PER_SPAN = 12;  // ~18.75% occupancy — higher chance of non-overlap
  std::mt19937 rng(42);

  std::vector<Span> spans(N_SPANS);
  for (int i = 0; i < N_SPANS; ++i) spans[i].id = i;

  // Sequential (non-random) allocation: all spans get same low offsets
  std::vector<Span> seq = spans;
  for (auto& s : seq) {
    for (int k = 0; k < ALLOC_PER_SPAN; ++k) s.live |= (1ULL << k);
  }

  // Random allocation: uniform random slots
  std::vector<Span> rnd = spans;
  for (auto& s : rnd) {
    std::vector<int> slots(Span::SLOTS);
    std::iota(slots.begin(), slots.end(), 0);
    std::shuffle(slots.begin(), slots.end(), rng);
    for (int k = 0; k < ALLOC_PER_SPAN; ++k) s.live |= (1ULL << slots[k]);
  }

  auto count_meshable = [](const std::vector<Span>& ss) {
    int pairs = 0;
    for (size_t i = 0; i < ss.size(); ++i)
      for (size_t j = i + 1; j < ss.size(); ++j)
        if ((ss[i].live & ss[j].live) == 0) ++pairs;
    return pairs;
  };

  int seq_pairs = count_meshable(seq);
  int rnd_pairs = count_meshable(rnd);

  // Simulate one mesh: pick first meshable pair in random, merge into one, "reclaim" the other
  int reclaimed = 0;
  uint64_t merged_live = 0;
  for (size_t i = 0; i < rnd.size() && !reclaimed; ++i) {
    for (size_t j = i + 1; j < rnd.size(); ++j) {
      if ((rnd[i].live & rnd[j].live) == 0) {
        merged_live = rnd[i].live | rnd[j].live;
        reclaimed = 1;
        break;
      }
    }
  }

  printf("{\n");
  printf("  \"probe\": \"mesh-mechanism\",\n");
  printf("  \"n_spans\": %d,\n", N_SPANS);
  printf("  \"slots_per_span\": %d,\n", Span::SLOTS);
  printf("  \"alloc_per_span\": %d,\n", ALLOC_PER_SPAN);
  print_json("sequential_meshable_pairs", seq_pairs);
  print_json("random_meshable_pairs", rnd_pairs);
  print_json("reclaimed_spans", reclaimed);
  print_json("merged_occupancy", reclaimed ? double(popcount(merged_live)) / Span::SLOTS : 0.0);
  printf("  \"note\": \"pure C++ simulation; addresses logical; not full Mesh\"\n");
  printf("}\n");

  return (rnd_pairs > seq_pairs) ? 0 : 1;  // mechanistic success if random better
}
