# Mesh-2019 Protocol

Protocol frozen for claims extraction and mechanism probe. Full app benchmarks pending machine and version pin.

## References & versions

- Paper: PLDI 2019, arXiv:1902.04738
- Implementation: plasma-umass/mesh (pin commit later)
- Baselines: jemalloc (paper used), glibc, modern mimalloc/tcmalloc for directional only

## Claims (same as README)

See README.md for numbered claims + failure criteria. Three tiers recorded separately:

- Exact: paper percentages (Firefox 16%, Redis 39%, etc.)
- Directional: Mesh lower RSS than non-meshing allocators under fragmentation-inducing workloads
- Mechanistic: savings vanish or drop when randomization or meshing disabled; addresses stable

## Workloads (round 1)

1. Pure C++ harness probe:
   - Create N spans of fixed size class
   - Allocate objects with random vs sequential offsets
   - Identify meshable pairs (non-overlapping live bits)
   - Perform or simulate remap + punch (or use real mmap/munmap on Linux)
   - Report: occupancy before/after, number of physical pages reclaimed, address equality of objects, time

2. Explicitly not in round 1:
   - Full Firefox Speedometer, Redis defrag, SPECint2006 matrix
   - Multi-thread high contention (later)

## Metrics & statistics

- RSS / VmRSS / physical pages via /proc or mallinfo-like
- Object address stability (pointer values before/after mesh)
- Meshing success rate, time for SplitMesher-like search
- Repeat: at least 5 runs, report median + min/max or IQR
- Counters: page faults, context switches if relevant (perf)

## Ablations

- Randomization on vs off
- Meshing enabled vs disabled
- Expected: random + mesh → lower footprint; no-mesh → higher

## Environment lock

To be filled in `lock/environment-paper.json` (paper machine, Mesh version, OS, compiler).

Modern differences noted but not used as excuse for Directional/Mechanistic failure.

## Next steps

- Implement `harness/mechanism_probe.cpp` (pure C++, no external Mesh dep for the probe itself)
- Pin official Mesh commit
- Only then expand to real workloads
