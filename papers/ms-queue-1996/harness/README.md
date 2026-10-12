# MS-queue-1996 harness

Minimal Michael-Scott lock-free queue + multi-thread probe.

- Dummy node
- Two-step enqueue (link + swing tail)
- Dequeue via CAS on head
- 4 producers × 4 consumers, verify sum

```bash
make -C papers/ms-queue-1996/harness
./papers/ms-queue-1996/harness/mechanism_probe
```

Toy only; no hazard pointers / epoch reclamation (safe in this single-process test because nodes are deleted only after successful CAS and no reuse).
