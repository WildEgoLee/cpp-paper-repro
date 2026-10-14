// EBR mechanism probe: why two epochs are required.
//
// Timeline (single-thread toy):
//   global = 0: enter → retire A (observed 0) → exit
//   advance → global = 1   A still in limbo (could still be seen by a thread in epoch 0)
//   advance → global = 2   A freeable (every thread has observed at least epoch 1)

#include "../model/minimal/ebr.hpp"
#include <cstdio>

int main() {
  EBR ebr;
  int dummy = 42;

  ebr.enter();
  ebr.retire(&dummy);
  ebr.exit();

  size_t limbo_after_retire = ebr.limbo_count();
  size_t reclaimed_after_retire = ebr.reclaimed_count();

  ebr.try_advance();                       // global → 1
  size_t limbo_after_1 = ebr.limbo_count();
  size_t reclaimed_after_1 = ebr.reclaimed_count();

  ebr.try_advance();                       // global → 2
  size_t limbo_after_2 = ebr.limbo_count();
  size_t reclaimed_after_2 = ebr.reclaimed_count();

  bool rule =
      limbo_after_retire == 1 &&
      reclaimed_after_retire == 0 &&
      limbo_after_1 == 1 &&
      reclaimed_after_1 == 0 &&
      limbo_after_2 == 0 &&
      reclaimed_after_2 == 1;

  std::printf("{\n");
  std::printf("  \"probe\": \"ebr-two-epoch\",\n");
  std::printf("  \"limbo_after_retire\": %zu,\n", limbo_after_retire);
  std::printf("  \"reclaimed_after_retire\": %zu,\n", reclaimed_after_retire);
  std::printf("  \"limbo_after_one_advance\": %zu,\n", limbo_after_1);
  std::printf("  \"reclaimed_after_one_advance\": %zu,\n", reclaimed_after_1);
  std::printf("  \"limbo_after_two_advances\": %zu,\n", limbo_after_2);
  std::printf("  \"reclaimed_after_two_advances\": %zu,\n", reclaimed_after_2);
  std::printf("  \"two_epoch_rule_holds\": %s,\n", rule ? "true" : "false");
  std::printf("  \"note\": \"retired in observed epoch e; freeable only at global >= e+2\"\n");
  std::printf("}\n");

  return rule ? 0 : 1;
}
