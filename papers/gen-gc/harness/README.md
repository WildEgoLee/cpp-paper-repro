# Generational GC harness

Minimal nursery + old + card-table write barrier.

- Allocations go into nursery
- Nursery full → minor GC
- Write barrier marks card dirty on old→young pointer
- Survivors can be promoted

```bash
make -C papers/gen-gc/harness
./papers/gen-gc/harness/mechanism_probe
```

Toy only; no real copying collector or precise roots.
