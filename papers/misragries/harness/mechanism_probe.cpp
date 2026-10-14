// Misra-Gries mechanism probe.
// Inserts a frequent item many times and rare items a few times.
// The frequent item should remain tracked.

#include "../model/minimal/mg.hpp"
#include <cstdio>

int main() {
  MisraGries mg(3);   // tracks at most 2 items

  // Frequent item: 7 appears 8 times.
  for (int i = 0; i < 8; ++i) mg.add(7);
  // Rare items: 1, 2, 3 appear once each.
  mg.add(1);
  mg.add(2);
  mg.add(3);

  bool frequent_tracked = mg.is_tracked(7);

  std::printf("{\n");
  std::printf("  \"probe\": \"misragries\",\n");
  std::printf("  \"frequent_tracked\": %s,\n", frequent_tracked ? "true" : "false");
  std::printf("  \"tracked_1\": %s,\n", mg.is_tracked(1) ? "true" : "false");
  std::printf("  \"note\": \"toy misra-gries; decrement all when full\"\n");
  std::printf("}\n");

  return frequent_tracked ? 0 : 1;
}
