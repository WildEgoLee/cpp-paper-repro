# CRDT harness

G-Counter with pointwise-max merge.

- Two replicas increment independently
- Mutual merge yields the same state
- Merge is commutative, associative, and idempotent

```bash
make -C papers/crdt/harness
./papers/crdt/harness/mechanism_probe
```

Toy only; demonstrates the join-semilattice property that makes state-based CRDTs work without coordination.
