// Data-oriented design probe: AoS vs SoA on a hot field.
// Sums only the x component; SoA should touch less memory.

#include "../model/minimal/layout.hpp"
#include <chrono>
#include <cstdio>
#include <vector>

int main() {
  constexpr size_t N = 1 << 20;          // 1M particles
  constexpr int kIters = 8;

  std::vector<ParticleAoS> aos(N);
  ParticlesSoA soa;
  soa.resize(N);

  for (size_t i = 0; i < N; ++i) {
    float v = static_cast<float>(i & 255);
    aos[i].x = aos[i].y = aos[i].z = aos[i].w = v;
    soa.x[i] = soa.y[i] = soa.z[i] = soa.w[i] = v;
  }

  // Warmup
  volatile float sink = 0;
  for (size_t i = 0; i < N; ++i) sink += aos[i].x + soa.x[i];

  auto time_aos = [&] {
    auto t0 = std::chrono::high_resolution_clock::now();
    float s = 0;
    for (int it = 0; it < kIters; ++it)
      for (size_t i = 0; i < N; ++i) s += aos[i].x;
    auto t1 = std::chrono::high_resolution_clock::now();
    sink += s;
    return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
  };

  auto time_soa = [&] {
    auto t0 = std::chrono::high_resolution_clock::now();
    float s = 0;
    for (int it = 0; it < kIters; ++it)
      for (size_t i = 0; i < N; ++i) s += soa.x[i];
    auto t1 = std::chrono::high_resolution_clock::now();
    sink += s;
    return std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
  };

  long aos_us = time_aos();
  long soa_us = time_soa();

  std::printf("{\n");
  std::printf("  \"probe\": \"dod-soa\",\n");
  std::printf("  \"particles\": %zu,\n", N);
  std::printf("  \"iters\": %d,\n", kIters);
  std::printf("  \"aos_us\": %ld,\n", aos_us);
  std::printf("  \"soa_us\": %ld,\n", soa_us);
  std::printf("  \"soa_faster\": %s,\n", soa_us < aos_us ? "true" : "false");
  std::printf("  \"note\": \"hot path reads only x; SoA keeps cache lines useful\"\n");
  std::printf("}\n");

  return 0;
}
