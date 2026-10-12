# Copy-on-Write harness

Minimal CoW page + isolation probe.

- Two holders share a page
- One writes; the other still sees the original
- After write the writer has an exclusive copy

```bash
make -C papers/cow/harness
./papers/cow/harness/mechanism_probe
```

Toy only; no page-table permissions or real page faults.
