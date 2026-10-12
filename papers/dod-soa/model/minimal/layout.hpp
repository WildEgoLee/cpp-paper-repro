// Minimal AoS vs SoA layouts for learning.

#pragma once
#include <vector>
#include <cstddef>

struct ParticleAoS {
  float x, y, z, w;
};

struct ParticlesSoA {
  std::vector<float> x, y, z, w;

  void resize(size_t n) {
    x.resize(n); y.resize(n); z.resize(n); w.resize(n);
  }
  size_t size() const { return x.size(); }
};
