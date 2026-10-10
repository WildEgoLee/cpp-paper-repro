# Mesh-2019 harness

Pure C++ mechanism probe. Does **not** link official Mesh.

Isolates:

- Random vs sequential allocation inside fixed-size page spans
- Detection of non-overlapping live objects (meshable pairs)
- **Real virtual-memory meshing**: `memfd_create` + `mmap(MAP_FIXED|MAP_SHARED)` so both virtual addresses see the same physical content, then `fallocate(FALLOC_FL_PUNCH_HOLE)` to reclaim the unused physical page
- Address stability (object VAs never change) and content identity after remap

Build & run:

```bash
make -C papers/mesh-2019/harness
./papers/mesh-2019/harness/mechanism_probe
```

Output: JSON to stdout. Requires Linux (memfd_create + fallocate punch hole). Not a full paper reproduction.
