# Mesh-2019 harness

Pure C++ mechanism probe. Does **not** link official Mesh. Isolates:

- Random vs sequential allocation inside fixed-size spans
- Detection of non-overlapping live objects (meshable pairs)
- Simulated compaction (merge live bits, reclaim one span)
- Address stability (logical object IDs stay the same)

Build & run:

```bash
make -C papers/mesh-2019/harness
./papers/mesh-2019/harness/mechanism_probe
```

Output: JSON to stdout. Not a full paper reproduction.
