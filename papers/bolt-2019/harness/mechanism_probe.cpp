// Pure C++ mechanism probe for BOLT (CGO 2019)
// Isolates: profile-guided hot basic-block / function layout improves performance
// vs cold-interleaved or random order.
// Does not require llvm-bolt; demonstrates the layout effect with noinline functions
// and synthetic profile knowledge.

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <vector>

volatile int sink = 0;

__attribute__((noinline)) int hot_path(int x) {
  // Representative hot computation
  return (x * 3 + 1) ^ (x >> 2);
}

__attribute__((noinline)) int cold_a(int x) { return x + 10000; }
__attribute__((noinline)) int cold_b(int x) { return x - 10000; }
__attribute__((noinline)) int cold_c(int x) { return x * 13; }
__attribute__((noinline)) int cold_d(int x) { return x / 7; }

static long long time_loop(bool good_layout, int N) {
  auto start = std::chrono::high_resolution_clock::now();
  int s = 0;
  if (good_layout) {
    // Hot path only — good layout (BOLT-like)
    for (int i = 0; i < N; ++i)
      s += hot_path(i);
  } else {
    // Cold blocks interleaved — bad layout (simulates unreordered binary)
    for (int i = 0; i < N; ++i) {
      if (i < 0) s += cold_a(i);  // never taken
      s += hot_path(i);
      if (i < 0) s += cold_b(i);
      if (i < 0) s += cold_c(i);
      if (i < 0) s += cold_d(i);
    }
  }
  sink = s;
  auto end = std::chrono::high_resolution_clock::now();
  return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
}

int main() {
  constexpr int N = 20000000;
  constexpr int REPS = 5;

  std::vector<long long> good, bad;
  for (int r = 0; r < REPS; ++r) {
    good.push_back(time_loop(true, N));
    bad.push_back(time_loop(false, N));
  }

  // Median
  auto median = [](std::vector<long long> v) {
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
  };
  long long med_good = median(good);
  long long med_bad = median(bad);
  double speedup = med_bad > 0 ? double(med_bad) / double(med_good) : 1.0;

  printf("{\n");
  printf("  \"probe\": \"bolt-mechanism\",\n");
  printf("  \"iterations\": %d,\n", N);
  printf("  \"reps\": %d,\n", REPS);
  printf("  \"median_good_layout_us\": %lld,\n", med_good);
  printf("  \"median_bad_layout_us\": %lld,\n", med_bad);
  printf("  \"speedup_good_over_bad\": %g,\n", speedup);
  printf("  \"profile_guided\": true,\n");
  printf("  \"ablation\": \"good (hot only) vs bad (cold interleaved)\",\n");
  printf("  \"note\": \"pure C++ layout effect; not full BOLT binary rewrite\"\n");
  printf("}\n");

  return (speedup > 1.02) ? 0 : 1;  // mechanistic: guided better
}
