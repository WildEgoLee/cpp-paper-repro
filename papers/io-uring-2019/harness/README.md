# io_uring-2019 harness

Two probes:

1. `mechanism_probe` — pure user-space simulated SQ/CQ rings.
2. `real_uring_probe` — **real** Linux io_uring via raw syscalls + mmap.
   - `io_uring_setup`
   - mmap SQ ring, CQ ring, SQEs
   - Submit `IORING_OP_NOP`
   - `io_uring_enter` and reap CQE

```bash
make -C papers/io-uring-2019/harness
./papers/io-uring-2019/harness/mechanism_probe
./papers/io-uring-2019/harness/real_uring_probe   # Linux with io_uring
```

Real probe requires a kernel that supports io_uring (most modern ones do).
