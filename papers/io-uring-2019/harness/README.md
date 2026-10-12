# io_uring-2019 harness

Three probes:

1. `mechanism_probe` — pure user-space simulated SQ/CQ rings.
2. `real_uring_probe` — real Linux io_uring, NOP only.
3. `real_uring_rw_probe` — real io_uring **file write + read**.
   - `io_uring_setup` + mmap
   - `IORING_OP_WRITE` then `IORING_OP_READ`
   - verifies data round-trip

```bash
make -C papers/io-uring-2019/harness
./papers/io-uring-2019/harness/mechanism_probe
./papers/io-uring-2019/harness/real_uring_probe
./papers/io-uring-2019/harness/real_uring_rw_probe
```

Real probes require a kernel with io_uring support.
