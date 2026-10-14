# Cuckoo Hashing harness

## Basic kick-out

`mechanism_probe` inserts two keys that share a slot. The second kicks the first to its other table. Lookup still finds both.

```bash
make -C papers/cuckoo/harness mechanism_probe
./papers/cuckoo/harness/mechanism_probe
```

## Rehash on cycle

`rehash_probe` inserts into a tiny table until the kick limit is hit. The table is rebuilt at double capacity and all keys remain findable.

```bash
make -C papers/cuckoo/harness rehash_probe
./papers/cuckoo/harness/rehash_probe
```

Toys only. They demonstrate the kick-out primitive and the rebuild path that real implementations need when a cycle appears.
