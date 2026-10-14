// Count Sketch mechanism probe.
// Adds keys with known frequencies, then estimates.
// Estimate should be close to the true frequency.

#include "../model/minimal/cs.hpp"
#include <cstdio>
#include <cmath>

int main() {
  CountSketch cs(5, 32);

  // Key 7 appears 6 times; key 3 appears 2 times.
  for (int i = 0; i < 6; ++i) cs.add(7);
  for (int i = 0; i < 2; ++i) cs.add(3);

  int est7 = cs.estimate(7);
  int est3 = cs.estimate(3);

  bool close = std::abs(est7 - 6) <= 4 && std::abs(est3 - 2) <= 4;

  std::printf("{\n");
  std::printf("  \"probe\": \"countsketch\",\n");
  std::printf("  \"true_7\": 6,\n");
  std::printf("  \"estimate_7\": %d,\n", est7);
  std::printf("  \"true_3\": 2,\n");
  std::printf("  \"estimate_3\": %d,\n", est3);
  std::printf("  \"close_enough\": %s,\n", close ? "true" : "false");
  std::printf("  \"note\": \"toy count sketch; sign hash + median\"\n");
  std::printf("}\n");

  return close ? 0 : 1;
}
