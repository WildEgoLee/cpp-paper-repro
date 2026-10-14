// Swiss Table mechanism probe.
// Inserts keys, then looks them up.
// The probe counts how many control bytes were examined.

#include "../model/minimal/swiss.hpp"
#include <cstdio>

int main() {
  SwissTable t(16);

  t.insert(10);
  t.insert(20);
  t.insert(30);

  bool found = t.contains(10) && t.contains(20) && t.contains(30);
  bool missing = !t.contains(99);
  size_t scans = t.groups_scanned();

  bool ok = found && missing && scans > 0;

  std::printf("{\n");
  std::printf("  \"probe\": \"swiss\",\n");
  std::printf("  \"contains_all\": %s,\n", found ? "true" : "false");
  std::printf("  \"missing_absent\": %s,\n", missing ? "true" : "false");
  std::printf("  \"control_scans\": %zu,\n", scans);
  std::printf("  \"note\": \"toy swiss; control bytes filter before key compare\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
