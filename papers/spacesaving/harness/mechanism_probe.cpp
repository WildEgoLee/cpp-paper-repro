// Space-Saving mechanism probe.
// Inserts a heavy hitter many times and light items a few times.
// The heavy hitter should remain tracked.

#include "../model/minimal/ss.hpp"
#include <cstdio>

int main() {
  SpaceSaving ss(3);

  // Heavy hitter: 7 appears 10 times.
  for (int i = 0; i < 10; ++i) ss.add(7);
  // Light items: 1, 2, 3 appear once each.
  ss.add(1);
  ss.add(2);
  ss.add(3);

  bool heavy_tracked = ss.is_tracked(7);
  int est7 = ss.estimate(7);

  bool ok = heavy_tracked && est7 >= 10;

  std::printf("{\n");
  std::printf("  \"probe\": \"spacesaving\",\n");
  std::printf("  \"heavy_tracked\": %s,\n", heavy_tracked ? "true" : "false");
  std::printf("  \"estimate_7\": %d,\n", est7);
  std::printf("  \"tracked_1\": %s,\n", ss.is_tracked(1) ? "true" : "false");
  std::printf("  \"note\": \"toy space-saving; min counter is replaced\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
