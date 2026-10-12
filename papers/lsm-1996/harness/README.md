# LSM-1996 harness

Minimal in-memory LSM toy.

- MemTable (std::map)
- Flush to simulated SSTable when size limit reached
- Get searches MemTable then L0 newest-first
- Newer version of a key wins

```bash
make -C papers/lsm-1996/harness
./papers/lsm-1996/harness/mechanism_probe
```

Toy only; no real disk, WAL, Bloom, or multi-level compaction yet.
