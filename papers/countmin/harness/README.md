# Count-Min Sketch harness

Minimal frequency estimator.

- d rows, w columns
- Each row has its own hash function
- Estimate is the minimum across rows (never underestimates)

```bash
make -C papers/countmin/harness
./papers/countmin/harness/mechanism_probe
```

Toy only; demonstrates the multi-row min primitive. Real CMS tunes d and w for error bounds.
