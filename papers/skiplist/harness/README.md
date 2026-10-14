# Skip List harness

Minimal skip list with random heights.

- Insert chooses a random height
- Search starts high and drops down
- Some nodes are promoted above level 1

```bash
make -C papers/skiplist/harness
./papers/skiplist/harness/mechanism_probe
```

Toy only; demonstrates probabilistic balancing without rotations or page splits.
