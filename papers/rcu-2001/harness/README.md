# RCU-2001 harness

Minimal counter-based RCU toy + linked-list probe.

Demonstrates:

- Reader critical sections (nesting + GP snapshot)
- Writer publishes new head
- Old node deferred via `call_rcu` + `synchronize`
- Readers and writer run concurrently

```bash
make -C papers/rcu-2001/harness
./papers/rcu-2001/harness/mechanism_probe
```

Toy only; not production RCU. See `notes/architecture.md`.
