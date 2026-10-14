// Skip List mechanism probe.
// Inserts keys with random heights, then searches.
// Also checks that some nodes were promoted above level 1.

#include "../model/minimal/skiplist.hpp"
#include <cstdio>
#include <cstdlib>

int main() {
  std::srand(42);
  SkipList s;

  for (int k : {10, 20, 5, 30, 15, 25, 1, 40}) s.insert(k);

  bool found = s.contains(20) && s.contains(1) && s.contains(40);
  bool missing = !s.contains(99);
  bool has_tall = s.tall_nodes() > 0;
  bool multi_level = s.max_level() > 1;

  bool ok = found && missing && has_tall && multi_level;

  std::printf("{\n");
  std::printf("  \"probe\": \"skiplist\",\n");
  std::printf("  \"max_level\": %d,\n", s.max_level());
  std::printf("  \"tall_nodes\": %d,\n", s.tall_nodes());
  std::printf("  \"contains_inserted\": %s,\n", found ? "true" : "false");
  std::printf("  \"missing_absent\": %s,\n", missing ? "true" : "false");
  std::printf("  \"probabilistic_heights\": %s,\n", has_tall ? "true" : "false");
  std::printf("  \"note\": \"toy skip list; random height, multi-level forward pointers\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
