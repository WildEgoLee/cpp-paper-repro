# Mesh-2019 harness

Pure C++ mechanism probe. Does **not** link official Mesh.

Progressive fidelity:

- Random vs sequential allocation → meshable pairs (SplitMesher-style bounded probing)
- Real virtual-memory meshing: `memfd_create` + `mmap(MAP_FIXED|MAP_SHARED)` + `fallocate(PUNCH_HOLE)`
- `mprotect(PROT_READ)` window on source during copy (concurrency safety sketch)
- Multi-page spans, multiple meshes, RSS before/after via `getrusage`
- Address stability + content identity after remap

Build & run:

```bash
make -C papers/mesh-2019/harness
./papers/mesh-2019/harness/mechanism_probe
```

Output: JSON. Linux only. Still a mechanism probe, not a full allocator or paper reproduction.
