// EBR stall probe: a thread stuck in a critical section
// prevents epoch advance, so limbo grows without bound.
//
// Thread 0 enters and never exits.
// Thread 1 keeps retiring nodes and trying to advance.
// Because thread 0 is still in CS at the old epoch, advance fails
// and retired nodes stay in limbo.

#include "../model/minimal/ebr.hpp"
#include <cstdio>
#include <vector>

// Extend the toy EBR with a second "thread" for the probe.
// We reuse the single-thread model by manually holding the critical section.

int main() {
  EBR ebr;
  std::vector<int> nodes(8);

  // Thread A enters and stalls (never exits).
  ebr.enter();
  // Simulate: announced stays at current epoch, in_critical stays true.
  uint64_t stalled_epoch = ebr.global_epoch();

  // Meanwhile the "other" thread retires several nodes.
  // In the single-thread toy we just retire while the CS is held.
  for (size_t i = 0; i < nodes.size(); ++i)
    ebr.retire(&nodes[i]);

  size_t limbo_before = ebr.limbo_count();

  // Try to advance several times. Because we are still in CS
  // and have not announced a newer epoch, advance is blocked.
  int advances = 0;
  for (int i = 0; i < 4; ++i)
    if (ebr.try_advance()) ++advances;

  size_t limbo_after = ebr.limbo_count();
  size_t reclaimed = ebr.reclaimed_count();

  bool stalled =
      (advances == 0) &&
      (limbo_after == limbo_before) &&
      (reclaimed == 0) &&
      (ebr.global_epoch() == stalled_epoch);

  std::printf("{\n");
  std::printf("  \"probe\": \"ebr-stall\",\n");
  std::printf("  \"stalled_epoch\": %lu,\n", stalled_epoch);
  std::printf("  \"limbo_before_attempts\": %zu,\n", limbo_before);
  std::printf("  \"advances_succeeded\": %d,\n", advances);
  std::printf("  \"limbo_after_attempts\": %zu,\n", limbo_after);
  std::printf("  \"reclaimed\": %zu,\n", reclaimed);
  std::printf("  \"stall_blocks_reclamation\": %s,\n", stalled ? "true" : "false");
  std::printf("  \"note\": \"thread stuck in CS at old epoch prevents advance; limbo unbounded\"\n");
  std::printf("}\n");

  return stalled ? 0 : 1;
}
