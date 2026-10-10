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

覆盖：source → compiler → binary → runtime → virtual memory。

## 现在就能跑的探针

```bash
make -C papers/mimalloc-2019/harness && ./papers/mimalloc-2019/harness/mechanism_probe
make -C papers/mesh-2019/harness && ./papers/mesh-2019/harness/mechanism_probe
make -C papers/bolt-2019/harness && ./papers/bolt-2019/harness/mechanism_probe
```

## 新增论文的标准

不为数量。每篇必须引入一种新的 architecture primitive（例如 LSM、work-stealing、RCU、io_uring、lock-free queue、data-oriented design）。

复制 [papers/_template](papers/_template)，并必须先写 `notes/architecture.md`。
