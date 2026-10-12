// Generational GC mechanism probe.

#include "../model/minimal/gc.hpp"
#include <cstdio>

int main() {
  GenGC gc;

  // Fill nursery several times, triggering minor GCs
  for (int i = 0; i < 12; ++i) gc.alloc();

  // Simulate an old→young write (marks card)
  gc.write_old_to_young();

  std::printf("{\n");
  std::printf("  \"probe\": \"gen-gc\",\n");
  std::printf("  \"nursery_used\": %zu,\n", gc.nursery_used());
  std::printf("  \"old_used\": %zu,\n", gc.old_used());
  std::printf("  \"dirty_cards\": %zu,\n", gc.dirty_cards());
  std::printf("  \"minor_gcs\": %zu,\n", gc.minor_gcs());
  std::printf("  \"promoted\": %s,\n", gc.old_used() > 0 ? "true" : "false");
  std::printf("  \"note\": \"toy generational GC; counts + card-table write barrier\"\n");
  std::printf("}\n");

  return gc.minor_gcs() > 0 && gc.old_used() > 0 ? 0 : 1;
}
