# HyperLogLog harness

Minimal cardinality estimator.

- Each key updates one register with its leading-zero count
- Estimate is the harmonic mean of the registers, scaled

```bash
make -C papers/hyperloglog/harness
./papers/hyperloglog/harness/mechanism_probe
```

Toy only; demonstrates the leading-zero + harmonic-mean primitive. Real HLL uses more registers and bias correction.
