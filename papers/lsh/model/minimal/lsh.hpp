// Minimal LSH interfaces.
// Banding: candidate if at least one band is identical.

#pragma once
#include <cstddef>
#include <vector>

class LSH {
public:
  // bands * rows = signature length.
  LSH(size_t bands, size_t rows);

  // True if the two signatures share at least one identical band.
  bool is_candidate(const std::vector<unsigned>& a,
                    const std::vector<unsigned>& b) const;

private:
  size_t bands_, rows_;
};
