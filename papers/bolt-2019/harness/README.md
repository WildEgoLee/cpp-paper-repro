# BOLT-2019 harness

Pure C++ mechanism probe. Does **not** require llvm-bolt.

Isolates the core layout effect:

- Profile-guided (hot path only) vs bad layout (cold blocks interleaved in the hot loop)
- Measures median wall time over several reps
- Ablation: good layout should be faster

Build & run:

```bash
make -C papers/bolt-2019/harness
./papers/bolt-2019/harness/mechanism_probe
```

Output: JSON. Demonstrates why post-link reordering of hot basic blocks helps; not a full BOLT reproduction.
