# Work-stealing-1994 harness

Minimal work-stealing deque + multi-worker probe.

- Owner: `push_bottom` / `pop_bottom` (LIFO)
- Thief: `steal_top` (FIFO)
- 4 workers, each seeded with 1000 tasks; empty workers steal from a random victim

```bash
make -C papers/work-stealing-1994/harness
./papers/work-stealing-1994/harness/mechanism_probe
```

Toy only; mutex-protected deque (real impl uses CAS).
