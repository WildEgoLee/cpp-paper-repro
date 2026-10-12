// Minimal SIMD interfaces.
// Scalar vs manually unrolled (simulating SIMD width).

#pragma once
#include <cstddef>
#include <vector>

// Scalar sum
float sum_scalar(const float* a, size_t n);

// Manually unrolled by 8 (simulates AVX width without intrinsics)
float sum_unrolled8(const float* a, size_t n);
