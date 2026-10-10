# BOLT-2019 Protocol

Protocol frozen for claims extraction. Full data-center / compiler rebuilds pending.

## References & versions

- Paper: CGO 2019, arXiv:1807.06735
- Implementation: now part of LLVM (bolt/); original facebookincubator/BOLT archived
- Baselines: binaries built with -O3, LTO, FDO/PGO vs same + BOLT

## Claims

See README.md. Tiers:

- Exact: paper speedups (up to 7% datacenter, 20%+ compilers) — not round-1 targets
- Directional: BOLT-optimized binary faster than non-BOLT on profiled workload
- Mechanistic: gains disappear or shrink when profile is random / hot-block reordering disabled

## Workloads (round 1)

1. Pure mechanism probe:
   - Small C++ microbenchmark with known hot/cold basic blocks
   - Collect simple profile (or synthetic)
   - Reorder blocks (or invoke llvm-bolt if available)
   - Measure cycles / I-cache misses before vs after

2. Explicitly not in round 1:
   - Full Clang/GCC bootstrap with BOLT
   - Facebook production workloads
   - Multi-architecture (focus x86-64 ELF)

## Metrics

- Wall time or cycles (perf or rdtsc)
- I-cache misses if available (perf stat)
- Repeat ≥5, report median

## Ablations

- Profile-guided reordering vs identity / random order
- Expected: guided → better; random → no gain or worse

## Next steps

- Implement harness/mechanism_probe (C++ microbench + optional llvm-bolt invocation)
- Pin LLVM version later
