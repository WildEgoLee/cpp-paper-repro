// Count-Min Sketch mechanism probe.
// Adds keys with known frequencies, then estimates.
// Estimate must be >= true frequency.

#include "../model/minimal/cms.hpp"
#include <cstdio>

int main() {
  CountMinSketch cms(4, 32);

  // Key 7 appears 5 times; key 3 appears 2 times.
  for (int i = 0; i < 5; ++i) cms.add(7);
  for (int i = 0; i < 2; ++i) cms.add(3);

  int est7 = cms.estimate(7);
  int est3 = cms.estimate(3);
  int est99 = cms.estimate(99);

  bool no_under = est7 >= 5 && est3 >= 2 && est99 >= 0;

  std::printf("{\n");
  std::printf("  \"probe\": \"countmin\",\n");
  std::printf("  \"true_7\": 5,\n");
  std::printf("  \"estimate_7\": %d,\n", est7);
  std::printf("  \"true_3\": 2,\n");
  std::printf("  \"estimate_3\": %d,\n", est3);
  std::printf("  \"estimate_absent\": %d,\n", est99);
  std::printf("  \"never_underestimate\": %s,\n", no_under ? "true" : "false");
  std::printf("  \"note\": \"toy CMS; estimate is min across rows\"\n");
  std::printf("}\n");

  return no_under ? 0 : 1;
}
