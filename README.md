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
| [papers/stm](papers/stm) | STM | optimistic concurrency / read-write sets |
| [papers/persistent](papers/persistent) | Persistent DS | path copying / structural sharing |
| [papers/gen-gc](papers/gen-gc) | Generational GC | write barrier / nursery / promotion |
| [papers/raft-2014](papers/raft-2014) | Raft, ATC 2014 | consensus / leader election / log replication |
| [papers/simd](papers/simd) | SIMD | data-parallel / vector registers |
| [papers/cow](papers/cow) | Copy-on-Write | delayed copy / shared read-only |
| [papers/ebr](papers/ebr) | EBR | epoch-based reclamation |
| [papers/crdt](papers/crdt) | CRDT | conflict-free replication |
| [papers/bptree](papers/bptree) | B+ Tree | ordered index / leaf chain |
| [papers/skiplist](papers/skiplist) | Skip List | probabilistic balancing |
| [papers/bloom](papers/bloom) | Bloom Filter | probabilistic membership |
| [papers/cuckoo](papers/cuckoo) | Cuckoo Hashing | kick-out / worst-case O(1) lookup |
| [papers/robinhood](papers/robinhood) | Robin Hood Hashing | variance-bounded probing |
| [papers/swiss](papers/swiss) | Swiss Table | control bytes / group probing |
| [papers/hyperloglog](papers/hyperloglog) | HyperLogLog | cardinality estimation |

覆盖：source → compiler → binary → runtime → virtual memory → concurrency → storage → scheduling → async I/O → lock-free structures → data layout → message-passing → user-level scheduling → optimistic concurrency → immutability → generational collection → distributed consensus → data parallelism → delayed copy → epoch reclamation → conflict-free replication → ordered index → probabilistic balancing → probabilistic membership → kick-out hashing → variance-bounded probing → control-byte group probing → cardinality estimation。

## 现在就能跑的探针

```bash
make -C papers/mimalloc-2019/harness && ./papers/mimalloc-2019/harness/mechanism_probe
make -C papers/mesh-2019/harness && ./papers/mesh-2019/harness/mechanism_probe
make -C papers/bolt-2019/harness && ./papers/bolt-2019/harness/mechanism_probe
make -C papers/rcu-2001/harness && ./papers/rcu-2001/harness/mechanism_probe
make -C papers/lsm-1996/harness && ./papers/lsm-1996/harness/mechanism_probe
make -C papers/io-uring-2019/harness && ./papers/io-uring-2019/harness/mechanism_probe
make -C papers/ms-queue-1996/harness && ./papers/ms-queue-1996/harness/mechanism_probe
make -C papers/work-stealing-1994/harness && ./papers/work-stealing-1994/harness/mechanism_probe
make -C papers/dod-soa/harness && ./papers/dod-soa/harness/mechanism_probe
make -C papers/actor-1973/harness && ./papers/actor-1973/harness/mechanism_probe
make -C papers/fibers/harness && ./papers/fibers/harness/mechanism_probe
make -C papers/gen-gc/harness && ./papers/gen-gc/harness/mechanism_probe
make -C papers/cow/harness && ./papers/cow/harness/mechanism_probe
make -C papers/ebr/harness && ./papers/ebr/harness/mechanism_probe
make -C papers/crdt/harness && ./papers/crdt/harness/mechanism_probe
make -C papers/bptree/harness && ./papers/bptree/harness/mechanism_probe
make -C papers/skiplist/harness && ./papers/skiplist/harness/mechanism_probe
make -C papers/bloom/harness && ./papers/bloom/harness/mechanism_probe
make -C papers/cuckoo/harness && ./papers/cuckoo/harness/mechanism_probe
make -C papers/robinhood/harness && ./papers/robinhood/harness/mechanism_probe
make -C papers/swiss/harness && ./papers/swiss/harness/mechanism_probe
```

## 新增论文的标准

不为数量。每篇必须引入一种新的 architecture primitive。

复制 [papers/_template](papers/_template)，并必须先写 `notes/architecture.md`。
