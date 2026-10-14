// Bloom Filter mechanism probe.
// Inserts a few keys, checks that they are reported present,
// and that some absent keys are correctly rejected.
// Also shows that false positives are possible.

#include "../model/minimal/bloom.hpp"
#include <cstdio>

int main() {
  BloomFilter bf(64, 3);

  for (int k : {10, 20, 30, 40}) bf.insert(k);

  bool inserted_ok = bf.may_contain(10) && bf.may_contain(20)
                  && bf.may_contain(30) && bf.may_contain(40);

  // Probe a range of keys that were never inserted.
  int false_positives = 0;
  int true_negatives = 0;
  for (int k = 100; k < 200; ++k) {
    if (bf.may_contain(k)) ++false_positives;
    else ++true_negatives;
  }

  bool no_false_neg = inserted_ok;
  bool has_true_neg = true_negatives > 0;

  bool ok = no_false_neg && has_true_neg;

  std::printf("{\n");
  std::printf("  \"probe\": \"bloom\",\n");
  std::printf("  \"bits\": %zu,\n", bf.bit_count());
  std::printf("  \"set_bits\": %zu,\n", bf.set_bits());
  std::printf("  \"inserted_reported\": %s,\n", no_false_neg ? "true" : "false");
  std::printf("  \"true_negatives\": %d,\n", true_negatives);
  std::printf("  \"false_positives\": %d,\n", false_positives);
  std::printf("  \"note\": \"no false negatives; false positives possible\"\n");
  std::printf("}\n");

  return ok ? 0 : 1;
}
