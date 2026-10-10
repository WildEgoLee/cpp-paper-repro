# Mesh: Compacting Memory Management for C/C++ Applications

Bobby Powers, David Tench, Emery D. Berger, Andrew McGregor. *Mesh: Compacting Memory Management for C/C++ Applications*. PLDI 2019.

- Official PDF: https://arxiv.org/pdf/1902.04738
- Official repo: https://github.com/plasma-umass/mesh
- Paper version / tag: master around 2019 (see lock later)

**Status**: `protocol-frozen` (claims extracted; harness probe to follow; full benchmark not yet run).

## Claims to answer (numbered, with failure criteria)

1. **Compaction without relocation**: Mesh reduces fragmentation / RSS by remapping virtual pages onto shared physical pages (meshing) while keeping object virtual addresses unchanged. Failure: any probe or run shows object addresses change, or no physical page reclamation observed under controlled fragmentation.

2. **Randomization enables high-probability meshing**: Uniform random allocation within spans makes it unlikely that live objects occupy the same offsets across spans, allowing pairs to be meshed. Failure: deterministic (non-random) allocation shows significantly lower meshing success rate than random under the same occupancy.

3. **Memory savings on real apps**: Mesh reduces memory consumption of Firefox by ~16% and Redis by ~39% relative to baseline (jemalloc etc.) with low runtime overhead. Failure (Exact): modern numbers deviate wildly without directional agreement; (Directional) no reduction or increase in RSS under similar workloads; (Mechanistic) savings disappear when meshing is disabled.

4. **Performance parity**: Runtime overhead is small (e.g. < few % on SPECint, Firefox Speedometer). Failure: large consistent slowdowns unrelated to meshing work.

## First-round experiments

- Mechanism probe (pure C++ harness, no full Mesh link): demonstrate randomized allocation + simulated / real virtual-memory meshing of two spans, measure occupancy before/after, address stability.
- Do **not** yet run full Firefox/Redis/SPECint matrix (requires pinning Mesh version, large binaries, multi-core machine matching paper).
- Ablation: randomization on/off, meshing on/off.

## Environment differences (part of design)

- 2026 CPU / kernel / glibc vs 2019 paper machines.
- Huge pages, THP, and modern ASLR may interact with Mesh's file-backed arena + punch-hole.
- Exact paper machine details to be locked in `lock/environment-paper.json`.

Protocol details in PROTOCOL.md. Do not commit result JSONs until protocol is fully frozen and runs complete.
