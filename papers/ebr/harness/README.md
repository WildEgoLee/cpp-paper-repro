# EBR harness

Minimal epoch-based reclamation + two-epoch rule probe.

- Retire a node in epoch 0
- After one advance (epoch 1) it is still in limbo
- After two advances (epoch 2) it is reclaimed

```bash
make -C papers/ebr/harness
./papers/ebr/harness/mechanism_probe
```

Toy only; single thread, no real concurrent readers.
Demonstrates why the protocol waits two epochs.
