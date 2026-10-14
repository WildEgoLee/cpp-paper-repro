// B+ Tree mechanism probe.
// Inserts keys, forces leaf splits, then does a range scan
// that walks the leaf chain.

#include "../model/minimal/bptree.hpp"
#include <cstdio>

int main() {
  BPTree t;
  // Insert enough to force splits (capacity = 3).
  for (int k : {10, 30, 20, 50, 40, 5, 60}) t.insert(k, k * 10);

  auto v30 = t.lookup(30);
  auto missing = t.lookup(99);
  auto rng = t.range(20, 50);

  bool ok = v30 && *v30 == 300 && !missing && rng.size() == 4
            && t.leaf_count() > 1;

  std::printf("{\n");
  std::printf("  \"probe\": \"bptree\",\n");
  std::printf("  \"leaf_count\": %zu,\n", t.leaf_count());
  std::printf("  \"lookup_30\": %s,\n", v30 ? "true" : "false");
  std::printf("  \"range_20_50_size\": %zu,\n", rng.size());
  std::printf("  \"splits_happened\": %s,\n", t.leaf_count() > 1 ? "true" : "false");
  std::printf("  \"note\": \"toy B+ tree; leaf chain enables range scan after one positioning\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
