// Counting Bloom Filter probe.
// Insert, check present, erase, check absent.

#include "../model/minimal/counting.hpp"
#include <cstdio>

int main() {
  CountingBloom cb(64, 3);

  cb.insert(42);
  bool before = cb.may_contain(42);

  cb.erase(42);
  bool after = cb.may_contain(42);

  // Re-insert to show it can come back.
  cb.insert(42);
  bool again = cb.may_contain(42);

  bool ok = before && !after && again;

  std::printf("{\n");
  std::printf("  \"probe\": \"counting-bloom\",\n");
  std::printf("  \"present_after_insert\": %s,\n", before ? "true" : "false");
  std::printf("  \"absent_after_erase\": %s,\n", !after ? "true" : "false");
  std::printf("  \"present_after_reinsert\": %s,\n", again ? "true" : "false");
  std::printf("  \"note\": \"counters allow deletion; still no false negatives\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
