// Cuckoo Hashing mechanism probe.
// Inserts a few keys that share a slot so one kick happens,
// then verifies lookup still works.

#include "../model/minimal/cuckoo.hpp"
#include <cstdio>

int main() {
  CuckooHash h(8);

  // 1 and 9 both hash to slot 1 on h1.
  bool ok1 = h.insert(1);
  bool ok9 = h.insert(9);   // should kick 1 to its other table

  bool found = h.contains(1) && h.contains(9);
  bool missing = !h.contains(99);
  bool had_kicks = h.kicks() > 0;

  bool ok = ok1 && ok9 && found && missing && had_kicks;

  std::printf("{\n");
  std::printf("  \"probe\": \"cuckoo\",\n");
  std::printf("  \"insert_1\": %s,\n", ok1 ? "true" : "false");
  std::printf("  \"insert_9\": %s,\n", ok9 ? "true" : "false");
  std::printf("  \"kicks\": %zu,\n", h.kicks());
  std::printf("  \"contains_both\": %s,\n", found ? "true" : "false");
  std::printf("  \"missing_absent\": %s,\n", missing ? "true" : "false");
  std::printf("  \"had_kicks\": %s,\n", had_kicks ? "true" : "false");
  std::printf("  \"note\": \"toy cuckoo; 9 kicks 1 to its other table, lookup still O(1)\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
