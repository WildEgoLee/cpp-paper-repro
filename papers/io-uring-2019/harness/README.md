# io_uring-2019 harness

Probes:

1. `mechanism_probe` — pure user-space simulated SQ/CQ.
2. `real_uring_probe` — real io_uring NOP.
3. `real_uring_rw_probe` — real file write + read.
4. `real_uring_batch_probe` — **batch SQEs + registered buffers**.
   - `IORING_REGISTER_BUFFERS`
   - `IORING_OP_READ_FIXED` (32 × 4 KiB)
   - Compares wall time against traditional `pread`

```bash
make -C papers/io-uring-2019/harness
./papers/io-uring-2019/harness/real_uring_batch_probe
```

Requires Linux with io_uring.
