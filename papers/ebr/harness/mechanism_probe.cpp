// EBR mechanism probe: why two epochs are required.
//
// Scenario:
//   epoch 0: enter, retire node A, exit
//   advance → epoch 1   (A still in limbo: some thread might still be in epoch 0)
//   advance → epoch 2   (A is now freeable: every thread has observed at least epoch 1)

#include "../model/minimal/ebr.hpp"
#include <cstdio>

int main() {
  EBR ebr;
  int dummy = 42;

  // Epoch 0: retire A
  ebr.enter();
  ebr.retire(&dummy);
  ebr.exit();

  size_t after_retire = ebr.limbo_count();
  size_t reclaimed_0 = ebr.reclaimed_count();

  // Advance once: epoch 1. A is not yet freeable.
  ebr.try_advance();
  size_t after_1 = ebr.limbo_count();
  size_t reclaimed_1 = ebr.reclaimed_count();

  // Advance again: epoch 2. A becomes freeable.
  ebr.try_advance();
  size_t after_2 = ebr.limbo_count();
  size_t reclaimed_2 = ebr.reclaimed_count();

  bool two_epoch_rule =
      (after_retire == 1) &&
      (after_1 == 1) && (reclaimed_1 == 0) &&
      (after_2 == 0) && (reclaimed_2 == 1);

  std::printf("{\n");
  std::printf("  \"probe\": \"ebr-two-epoch\",\n");
  std::printf("  \"after_retire_limbo\": %zu,\n", after_retire);
  std::printf("  \"after_one_advance_limbo\": %zu,\n", after_1);
  std::printf("  \"after_one_advance_reclaimed\": %zu,\n", reclaimed_1);
  std::printf("  \"after_two_advances_limbo\": %zu,\n", after_2);
  std::printf("  \"after_two_advances_reclaimed\": %zu,\n", reclaimed_2);
  std::printf("  \"two_epoch_rule_holds\": %s,\n", two_epoch_rule ? "true" : "false");
  std::printf("  \"note\": \"node retired in e is freeable only at e+2\"\n");
  std::printf("}\n");

  return two_epoch_rule ? 0 : 1;
}
