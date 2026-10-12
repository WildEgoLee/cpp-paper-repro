# DOD-SoA harness

Compares Array-of-Structures vs Structure-of-Arrays on a hot field.

- 1M particles, each with x/y/z/w
- Hot path sums only `x`
- Reports wall time for both layouts

```bash
make -C papers/dod-soa/harness
./papers/dod-soa/harness/mechanism_probe
```

On a cold cache the SoA version typically touches 1/4 of the memory of AoS.
