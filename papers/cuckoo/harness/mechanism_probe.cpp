// Cuckoo Hashing mechanism probe.
// Inserts keys that share hash slots so kick-out happens.

#include "../model/minimal/cuckoo.hpp"
#include <cstdio>

int main() {
  CuckooHash h(8);

  // Keys that collide on h1 (mod 8).
  bool all_inserted = true;
  for (int k : {1, 9, 17, 2, 10}) {
    if (!h.insert(k)) all_inserted = false;
  }

  bool found = h.contains(1) && h.contains(9) && h.contains(17)
            && h.contains(2) && h.contains(10);
  bool missing = !h.contains(99);
  bool had_kicks = h.kicks() > 0;

  bool ok = all_inserted && found && missing && had_kicks;

  std::printf("{\n");
  std::printf("  \"probe\": \"cuckoo\",\n");
  std::printf("  \"all_inserted\": %s,\n", all_inserted ? "true" : "false");
  std::printf("  \"kicks\": %zu,\n", h.kicks());
  std::printf("  \"contains_inserted\": %s,\n", found ? "true" : "false");
  std::printf("  \"missing_absent\": %s,\n", missing ? "true" : "false");
  std::printf("  \"had_kicks\": %s,\n", had_kicks ? "true" : "false");
  std::printf("  \"note\": \"toy cuckoo; lookup is two probes, insert kicks on collision\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
