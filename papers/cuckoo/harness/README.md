# Cuckoo Hashing harness

Minimal two-table cuckoo hash.

- Lookup checks at most two slots
- Insert may kick the previous occupant to its other location
- Cycle is detected by a kick limit (rehash not implemented)

```bash
make -C papers/cuckoo/harness
./papers/cuckoo/harness/mechanism_probe
```

Toy only; demonstrates the kick-out primitive and worst-case constant lookup.
