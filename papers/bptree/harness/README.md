# B+ Tree harness

Minimal leaf-chain B+ Tree.

- Inserts force leaf splits
- Lookup finds a key
- Range scan walks the leaf chain once

```bash
make -C papers/bptree/harness
./papers/bptree/harness/mechanism_probe
```

Toy only; no internal nodes. The architecture point is the sorted leaf chain that turns a range scan into a sequential walk after the first leaf is found.
