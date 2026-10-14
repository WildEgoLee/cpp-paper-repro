# Misra-Gries harness

Minimal frequent-item tracker.

- Fixed counters (k-1)
- New item when full decrements all counters and drops zeros
- Frequent items stay tracked

```bash
make -C papers/misragries/harness
./papers/misragries/harness/mechanism_probe
```

Toy only; demonstrates the decrement-all primitive. Real Misra-Gries uses a map and can be merged.
