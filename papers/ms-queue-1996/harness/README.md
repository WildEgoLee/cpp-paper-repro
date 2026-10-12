# MS-queue-1996 harness

Minimal Michael-Scott lock-free queue + multi-thread probe.

ABA protection: **tagged pointers**.
On 64-bit the pointer word packs a 16-bit tag in the high bits
(valid because x86-64 user virtual addresses use ≤48 bits).
Every CAS on head/tail/next also increments the tag, so a recycled
node cannot make an old CAS succeed.

```bash
make -C papers/ms-queue-1996/harness
./papers/ms-queue-1996/harness/mechanism_probe
```

Still a toy (no hazard pointers). The tag prevents the classic ABA
on the pointer itself; safe reclamation of the deleted node still
requires additional techniques in a general setting.
