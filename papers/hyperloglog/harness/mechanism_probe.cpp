// HyperLogLog mechanism probe.
// Adds a known number of distinct keys, then estimates cardinality.
// The estimate should be in the same order of magnitude.

#include "../model/minimal/hll.hpp"
#include <cstdio>

int main() {
  HyperLogLog h(16);

  constexpr int N = 100;
  for (int i = 0; i < N; ++i) h.add(i);

  double est = h.estimate();
  bool close = est > N * 0.3 && est < N * 3.0;

  std::printf("{\n");
  std::printf("  \"probe\": \"hyperloglog\",\n");
  std::printf("  \"true_cardinality\": %d,\n", N);
  std::printf("  \"estimate\": %.1f,\n", est);
  std::printf("  \"order_of_magnitude\": %s,\n", close ? "true" : "false");
  std::printf("  \"note\": \"toy HLL; harmonic mean of leading zeros\"\n");
  std::printf("}\n");

  return close ? 0 : 1;
}
