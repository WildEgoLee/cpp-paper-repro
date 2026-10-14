# Count Sketch harness

Minimal unbiased frequency estimator.

- d rows, w columns
- Each row has a sign hash (+1/-1)
- Estimate is the median of the signed counters

```bash
make -C papers/countsketch/harness
./papers/countsketch/harness/mechanism_probe
```

Toy only; demonstrates the sign-hash + median primitive. Real Count Sketch tunes d and w for variance bounds.
