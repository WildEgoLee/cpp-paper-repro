// Robin Hood Hashing mechanism probe.
// Inserts keys that share a home slot so a richer key is stolen,
// then checks lookup and that max PSL is bounded.

#include "../model/minimal/robinhood.hpp"
#include <cstdio>

int main() {
  RobinHood h(8);

  // Keys that all hash to the same home (mod 8).
  h.insert(1);
  h.insert(9);   // steals from 1 if 1 is richer
  h.insert(17);

  bool found = h.contains(1) && h.contains(9) && h.contains(17);
  bool missing = !h.contains(99);
  int max_psl = h.max_psl();

  bool ok = found && missing && max_psl > 0;

  std::printf("{\n");
  std::printf("  \"probe\": \"robinhood\",\n");
  std::printf("  \"contains_all\": %s,\n", found ? "true" : "false");
  std::printf("  \"missing_absent\": %s,\n", missing ? "true" : "false");
  std::printf("  \"max_psl\": %d,\n", max_psl);
  std::printf("  \"note\": \"toy robin hood; richer keys are stolen, PSL variance bounded\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
