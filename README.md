# cpp-paper-repro

C++ / native 系统**架构学习实验室**。一篇论文一个目录。

目标不是严谨复现论文数字，而是：

```
paper → 自己画架构 + 写 invariants → 设计接口 → 实现 toy model → 再读上游 → 对比 tradeoffs
```

每篇论文必须带来一种以前没有的 architecture primitive。

## 三层

1. **Mechanism implementation** — 把论文机制拆成可独立实现的数据结构与快慢路径。
2. **System decomposition** — 把对象、ownership、VM/OS 边界分离清楚。
3. **Engineering architecture** — 实验规格、adapter、执行计划、结果解析等可复用框架。

## 系列（按架构层次）

| 目录 | 论文 | 学习的架构 |
| --- | --- | --- |
| [papers/thinlto-2017](papers/thinlto-2017) | ThinLTO, CGO 2017 | compiler summaries / incremental / caching |
| [papers/bolt-2019](papers/bolt-2019) | BOLT, CGO 2019 | binary CFG / profile / layout |
| [papers/mimalloc-2019](papers/mimalloc-2019) | Mimalloc, APLAS 2019 | ownership / sharding / fast-slow path |
| [papers/mesh-2019](papers/mesh-2019) | Mesh, PLDI 2019 | VM remapping / fragmentation |
| [papers/rcu-2001](papers/rcu-2001) | RCU, OLS 2001 | deferred reclamation / grace periods |
| [papers/lsm-1996](papers/lsm-1996) | LSM-tree, 1996 | memtable / SSTable / leveled compaction |
| [papers/work-stealing-1994](papers/work-stealing-1994) | Work-Stealing, FOCS 1994 | per-worker deque / steal / work-first |
| [papers/io-uring-2019](papers/io-uring-2019) | io_uring, 2019 | SQ/CQ shared rings / async I/O |
| [papers/ms-queue-1996](papers/ms-queue-1996) | MS Queue, PODC 1996 | lock-free FIFO / CAS / dummy node |
| [papers/dod-soa](papers/dod-soa) | Data-Oriented Design | SoA vs AoS / cache layout |
| [papers/actor-1973](papers/actor-1973) | Actor Model, IJCAI 1973 | message-passing / private state |
| [papers/fibers](papers/fibers) | Fibers | cooperative user-level multitasking |

覆盖：source → compiler → binary → runtime → virtual memory → concurrency → storage → scheduling → async I/O → lock-free structures → data layout → message-passing → user-level scheduling。

## 现在就能跑的探针

```bash
make -C papers/mimalloc-2019/harness && ./papers/mimalloc-2019/harness/mechanism_probe
make -C papers/mesh-2019/harness && ./papers/mesh-2019/harness/mechanism_probe
make -C papers/bolt-2019/harness && ./papers/bolt-2019/harness/mechanism_probe
make -C papers/rcu-2001/harness && ./papers/rcu-2001/harness/mechanism_probe
make -C papers/lsm-1996/harness && ./papers/lsm-1996/harness/mechanism_probe
make -C papers/io-uring-2019/harness && ./papers/io-uring-2019/harness/mechanism_probe
make -C papers/ms-queue-1996/harness && ./papers/ms-queue-1996/harness/mechanism_probe
make -C papers/dod-soa/harness && ./papers/dod-soa/harness/mechanism_probe
make -C papers/actor-1973/harness && ./papers/actor-1973/harness/mechanism_probe
```

## 新增论文的标准

不为数量。每篇必须引入一种新的 architecture primitive。

复制 [papers/_template](papers/_template)，并必须先写 `notes/architecture.md`。
