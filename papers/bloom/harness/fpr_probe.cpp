// Bloom Filter FPR growth probe.
// Inserts more and more keys and measures how many absent keys
// are incorrectly reported present.

#include "../model/minimal/bloom.hpp"
#include <cstdio>

int main() {
  constexpr size_t M = 256;
  constexpr size_t K = 3;
  BloomFilter bf(M, K);

  int inserted = 0;
  int fpr_at_32 = -1, fpr_at_64 = -1, fpr_at_128 = -1;

  auto measure = [&]() {
    int fp = 0;
    for (int k = 10000; k < 10100; ++k)
      if (bf.may_contain(k)) ++fp;
    return fp;
  };

  for (int k = 0; k < 128; ++k) {
    bf.insert(k);
    ++inserted;
    if (inserted == 32) fpr_at_32 = measure();
    if (inserted == 64) fpr_at_64 = measure();
    if (inserted == 128) fpr_at_128 = measure();
  }

  bool grows = fpr_at_32 < fpr_at_64 && fpr_at_64 <= fpr_at_128;

  std::printf("{\n");
  std::printf("  \"probe\": \"bloom-fpr\",\n");
  std::printf("  \"bits\": %zu,\n", M);
  std::printf("  \"k\": %zu,\n", K);
  std::printf("  \"fpr_after_32\": %d,\n", fpr_at_32);
  std::printf("  \"fpr_after_64\": %d,\n", fpr_at_64);
  std::printf("  \"fpr_after_128\": %d,\n", fpr_at_128);
  std::printf("  \"fpr_grows\": %s,\n", grows ? "true" : "false");
  std::printf("  \"note\": \"false positive rate rises as more bits are set\"\n");
  std::printf("}\n");

  return grows ? 0 : 1;
}
