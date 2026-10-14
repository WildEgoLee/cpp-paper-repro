# Space-Saving harness

Minimal heavy-hitter tracker.

- Fixed k counters
- New item replaces the minimum-count counter
- Heavy hitters stay tracked

```bash
make -C papers/spacesaving/harness
./papers/spacesaving/harness/mechanism_probe
```

Toy only; demonstrates the counter-replacement primitive. Real Space-Saving uses a heap to find the minimum quickly.
