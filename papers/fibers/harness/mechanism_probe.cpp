// Fiber mechanism probe: two fibers interleave via yield.

#include "../model/minimal/fiber.hpp"
#include <cstdio>
#include <string>
#include <vector>

int main() {
  Scheduler sched;
  std::vector<std::string> log;

  // Fiber A: prints A0 A1 A2, yielding between each
  int a_step = 0;
  sched.spawn([&](Fiber& f) -> bool {
    if (a_step >= 3) return false;
    log.push_back("A" + std::to_string(a_step));
    ++a_step;
    f.yield();
    return true;
  });

  // Fiber B: prints B0 B1 B2
  int b_step = 0;
  sched.spawn([&](Fiber& f) -> bool {
    if (b_step >= 3) return false;
    log.push_back("B" + std::to_string(b_step));
    ++b_step;
    f.yield();
    return true;
  });

  sched.run();

  // Expect interleaving: A0 B0 A1 B1 A2 B2 (or similar round-robin)
  bool interleaved = (log.size() == 6);
  int switches = 0;
  for (size_t i = 1; i < log.size(); ++i)
    if (log[i][0] != log[i-1][0]) ++switches;

  std::printf("{\n");
  std::printf("  \"probe\": \"fibers\",\n");
  std::printf("  \"steps\": %zu,\n", log.size());
  std::printf("  \"context_switches\": %d,\n", switches);
  std::printf("  \"total_yields\": %d,\n", sched.total_yields());
  std::printf("  \"interleaved\": %s,\n", (interleaved && switches >= 3) ? "true" : "false");
  std::printf("  \"log\": [\n");
  for (size_t i = 0; i < log.size(); ++i)
    std::printf("    \"%s\"%s\n", log[i].c_str(), i + 1 == log.size() ? "" : ",");
  std::printf("  ],\n");
  std::printf("  \"note\": \"cooperative toy; yield returns to scheduler (no real stack switch)\"\n");
  std::printf("}\n");

  return (interleaved && switches >= 3) ? 0 : 1;
}
