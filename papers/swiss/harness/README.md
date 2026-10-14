# Swiss Table harness

Minimal control-byte table.

- Control bytes are separate from keys
- Lookup scans control bytes first; only matching fingerprints load the real key
- Empty control byte stops the probe

```bash
make -C papers/swiss/harness
./papers/swiss/harness/mechanism_probe
```

Toy only; demonstrates the control-byte filter primitive. Real Swiss Tables use SIMD to scan 8/16 bytes at once.
