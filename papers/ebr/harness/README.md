# EBR harness

## Two-epoch rule

`mechanism_probe` retires a node in epoch 0 and shows it is freeable only after two advances.

```bash
make -C papers/ebr/harness mechanism_probe
./papers/ebr/harness/mechanism_probe
```

## Stall (unbounded limbo)

`stall_probe` holds a critical section and never exits. Subsequent `try_advance` calls fail and retired nodes stay in limbo.

```bash
make -C papers/ebr/harness stall_probe
./papers/ebr/harness/stall_probe
```

Both are single-thread toys. They demonstrate the protocol rules, not a concurrent runtime.
