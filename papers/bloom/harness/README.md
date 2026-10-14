# Bloom Filter harness

Minimal bit-array Bloom filter.

- Insert sets k bits
- Query returns false only if a bit is clear (no false negatives)
- Absent keys may still return true (false positives)

```bash
make -C papers/bloom/harness
./papers/bloom/harness/mechanism_probe
```

Toy only; demonstrates the core membership trade-off.
