# Fibers harness

Minimal cooperative scheduler + interleaving probe.

- Two fibers, each with 3 steps
- `yield` returns control to the scheduler
- Verifies round-robin interleaving

```bash
make -C papers/fibers/harness
./papers/fibers/harness/mechanism_probe
```

Toy only; no real stack switching (that needs `ucontext` or assembly).
The probe demonstrates the cooperative scheduling protocol.
