// Actor model mechanism probe.
// Two actors exchange messages; verify count and become.

#include "../model/minimal/actor.hpp"
#include <atomic>
#include <cstdio>
#include <string>
#include <thread>

int main() {
  std::atomic<int> pings{0};
  std::atomic<int> pongs{0};

  Actor* pong = nullptr;
  Actor ping([&](Actor& self, std::any msg) {
    if (auto* s = std::any_cast<std::string>(&msg)) {
      if (*s == "pong") {
        pongs.fetch_add(1, std::memory_order_relaxed);
        if (pongs.load() < 5 && pong) pong->send(std::string("ping"));
      }
    }
  });

  Actor pong_actor([&](Actor& self, std::any msg) {
    if (auto* s = std::any_cast<std::string>(&msg)) {
      if (*s == "ping") {
        pings.fetch_add(1, std::memory_order_relaxed);
        ping.send(std::string("pong"));
      }
    }
  });
  pong = &pong_actor;

  // Start the exchange
  pong_actor.send(std::string("ping"));

  // Drive both mailboxes until quiet
  for (int i = 0; i < 100; ++i) {
    bool progressed = false;
    progressed |= ping.process_one();
    progressed |= pong_actor.process_one();
    if (!progressed && pings.load() >= 5) break;
    std::this_thread::yield();
  }

  std::printf("{\n");
  std::printf("  \"probe\": \"actor-model\",\n");
  std::printf("  \"pings\": %d,\n", pings.load());
  std::printf("  \"pongs\": %d,\n", pongs.load());
  std::printf("  \"exchanged\": %s,\n", (pings.load() >= 5 && pongs.load() >= 5) ? "true" : "false");
  std::printf("  \"note\": \"toy actors with mutex mailboxes; single-threaded driver\"\n");
  std::printf("}\n");

  return (pings.load() >= 5 && pongs.load() >= 5) ? 0 : 1;
}
