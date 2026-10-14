// Cuckoo Hashing mechanism probe.
// Inserts keys (some cause kicks), then checks lookup.

#include "../model/minimal/cuckoo.hpp"
#include <cstdio>

int main() {
  CuckooHash h(16);

  bool all_inserted = true;
  for (int k : {10, 20, 30, 40, 50, 60}) {
    if (!h.insert(k)) all_inserted = false;
  }

  bool found = h.contains(10) && h.contains(30) && h.contains(60);
  bool missing = !h.contains(99);
  bool had_kicks = h.kicks() > 0;

  bool ok = all_inserted && found && missing;

  std::printf("{\n");
  std::printf("  \"probe\": \"cuckoo\",\n");
  std::printf("  \"all_inserted\": %s,\n", all_inserted ? "true" : "false");
  std::printf("  \"kicks\": %zu,\n", h.kicks());
  std::printf("  \"contains_inserted\": %s,\n", found ? "true" : "false");
  std::printf("  \"missing_absent\": %s,\n", missing ? "true" : "false");
  std::printf("  \"had_kicks\": %s,\n", had_kicks ? "true" : "false");
  std::printf("  \"note\": \"toy cuckoo; lookup is two probes, insert may kick\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
