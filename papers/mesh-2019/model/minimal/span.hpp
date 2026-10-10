// Minimal Mesh model — interfaces only, for learning architecture.
// Policy / Mechanism separation. No real mmap yet.

#pragma once
#include <cstdint>
#include <optional>
#include <vector>

struct OccupancyBitmap {
  uint64_t bits = 0;
  bool overlaps(const OccupancyBitmap& o) const { return (bits & o.bits) != 0; }
  bool meshable_with(const OccupancyBitmap& o) const { return !overlaps(o); }
};

struct Span {
  int id = 0;
  void* virtual_address = nullptr;
  OccupancyBitmap occupancy;
};

// Policy: which pair to mesh
class MeshPolicy {
public:
  virtual ~MeshPolicy() = default;
  virtual std::optional<std::pair<int, int>>
  find_pair(const std::vector<Span>& spans) = 0;
};

// Mechanism: how to actually remap / reclaim
class PageBackend {
public:
  virtual ~PageBackend() = default;
  virtual void remap(Span& keep, Span& remove) = 0;
  virtual void reclaim(Span& removed) = 0;
};

// Example policies (implement later)
class RandomMeshPolicy : public MeshPolicy {
public:
  std::optional<std::pair<int, int>> find_pair(const std::vector<Span>& spans) override;
};
