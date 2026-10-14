// Cuckoo rehash probe.
// Insert enough keys into a tiny table to force a rehash,
// then verify all keys are still findable.

#include "../model/minimal/cuckoo.hpp"
#include <cstdio>
#include <vector>

int main() {
  CuckooHash h(4);
  std::vector<int> keys = {1, 5, 9, 13, 17, 21};

  bool all_ok = true;
  for (int k : keys) {
    if (!h.insert(k)) all_ok = false;
  }

  bool found_all = true;
  for (int k : keys) {
    if (!h.contains(k)) found_all = false;
  }

  bool rehashed = h.rehashes() > 0;
  bool grew = h.capacity() > 4;

  bool ok = all_ok && found_all && rehashed && grew;

  std::printf("{\n");
  std::printf("  \"probe\": \"cuckoo-rehash\",\n");
  std::printf("  \"inserted\": %s,\n", all_ok ? "true" : "false");
  std::printf("  \"found_all\": %s,\n", found_all ? "true" : "false");
  std::printf("  \"rehashes\": %zu,\n", h.rehashes());
  std::printf("  \"final_capacity\": %zu,\n", h.capacity());
  std::printf("  \"kicks\": %zu,\n", h.kicks());
  std::printf("  \"note\": \"kick limit hit → rehash to larger tables\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
