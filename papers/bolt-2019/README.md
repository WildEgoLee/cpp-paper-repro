# BOLT: A Practical Binary Optimizer for Data Centers and Beyond

Maksim Panchenko, Rafael Auler, Bill Nell, Guilherme Ottoni. *BOLT: A Practical Binary Optimizer for Data Centers and Beyond*. CGO 2019.

- Official PDF: https://arxiv.org/pdf/1807.06735
- Original repo: https://github.com/facebookarchive/BOLT (now in LLVM)
- Current: https://github.com/llvm/llvm-project/tree/main/bolt

**Status**: `protocol-frozen` (claims extracted; mechanism probe to follow).

## Claims to answer (numbered, with failure criteria)

1. **Post-link layout optimization**: BOLT improves performance of already highly optimized binaries (FDO + LTO) by reordering basic blocks and functions based on sample profiles. Failure: no speedup or slowdown on profiled hot paths when layout is improved.

2. **Complementary to compiler opts**: Gains persist on top of LTO + FDO/PGO. Failure: BOLT adds nothing once LTO+FDO is applied, or requires source changes.

3. **Sample-based**: Uses sampling profiler (Linux perf) rather than instrumentation. Failure: requires full instrumentation or source-level changes to achieve gains.

4. **Practical scale**: Works on large data-center binaries and open-source compilers (Clang/GCC) with low overhead. Failure: only works on toy binaries or has prohibitive processing time.

## First-round experiments

- Mechanism probe (pure C++/LLVM BOLT or simple CFG reordering sketch): demonstrate that reordering hot basic blocks according to a profile reduces I-cache misses or improves a microbenchmark.
- Do **not** yet run full Facebook workloads or full Clang/GCC rebuilds.
- Ablation: profile-guided vs random/no reordering.

## Environment differences

- 2026 LLVM/BOLT vs 2019 Facebook version.
- Modern CPUs, kernels, and perf sampling may differ.
- Differences recorded; not excuses for Directional/Mechanistic failure.

Protocol in PROTOCOL.md.
