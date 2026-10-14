# Robin Hood Hashing harness

Minimal linear-probing Robin Hood table.

- Insert walks the probe sequence
- A key with larger PSL steals from a richer key
- Lookup can stop early when it sees a smaller PSL

```bash
make -C papers/robinhood/harness
./papers/robinhood/harness/mechanism_probe
```

Toy only; demonstrates the steal-from-the-rich primitive and bounded probe variance.
