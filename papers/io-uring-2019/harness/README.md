# io_uring-2019 harness

Minimal user-space simulation of SQ/CQ rings.

- `Ring<T>`: SPSC head/tail ring
- Probe: user pushes SQEs, simulated kernel thread consumes them and posts CQEs
- Verifies user_data and result echo

```bash
make -C papers/io-uring-2019/harness
./papers/io-uring-2019/harness/mechanism_probe
```

Toy only; no real `io_uring_setup` / mmap / kernel.
