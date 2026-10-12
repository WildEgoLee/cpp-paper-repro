# Actor-1973 harness

Minimal Actor model + message-passing probe.

- Private mailbox (mutex-protected queue)
- `send` / `become` / `process_one`
- Two actors exchange ping/pong messages

```bash
make -C papers/actor-1973/harness
./papers/actor-1973/harness/mechanism_probe
```

Toy only; single-threaded driver, no real scheduler or supervision.
