# Mimalloc: Free List Sharding in Action

状态：`protocol-frozen`。机制探针可编译。论文对照矩阵还没有跑。

Daan Leijen, Benjamin Zorn, Leonardo de Moura. *Mimalloc: Free List Sharding in Action*. APLAS 2019, LNCS 11893, pp. 244–265。技术报告 MSR-TR-2019-18。

- 论文：<https://www.microsoft.com/en-us/research/publication/mimalloc-free-list-sharding-in-action/>
- 实现：<https://github.com/microsoft/mimalloc>
- 论文写明使用的 tag：**v1.0.0**
- benchmark 套件：<https://github.com/daanx/mimalloc-bench>（用与 `v1.0.0` 同时代的提交，不要用今天 README 里的版本表代替论文）

## 这篇在回答什么

不是「某个应用怎么用 C++」。是 malloc/free 的组织结构：size class 下面是一条很长的 free list，还是大约 64KiB 的软件页各自带一条 free list（free list sharding）。页上还有两条延迟链表，用来把本线程释放和跨线程释放移出 fast path。

```text
传统：
  size class → 一条很长的 free list

mimalloc：
  size class → page A 的 free list
             → page B 的 free list
             → page C 的 free list

每个 page 上：
  free          分配 fast path 直接弹块
  local_free    本线程 free，先不放回 free
  thread_free   其他线程 free，原子压栈，拥有者在 slow path 取回
```

`local_free` 让 free list 会耗尽，于是分配每隔若干次必然走进 slow path。论文把这个性质叫做 temporal cadence：维护工作不必在每次 malloc 上做，也不会永远不做。

## 和「今天的 mimalloc」分开

现在的 mimalloc 仍然使用 free-list sharding，但代码已经不是 2019 年的 `v1.0.0`。仓库后续有 v1 / v2 / v3 几条维护线。

| 轨道 | 测的是 |
| --- | --- |
| Paper reproduction | tag `v1.0.0` |
| Modern replication | 当前推荐版本 |

两条轨道都要留。只跑当前版本，回答的是「2026 年的 mimalloc 快不快」，不是「2019 年的论文能不能复现」。

## 这个目录里现在有什么

| 路径 | 内容 |
| --- | --- |
| [PROTOCOL.md](PROTOCOL.md) | 冻结的 claim、96 组矩阵、ablation、验收判据 |
| [harness/](harness) | 不依赖上游库的机制探针 |
| [bench/matrix.json](bench/matrix.json) | 第一轮配置，状态是 `frozen-not-run` |
| [results/](results) | 一次 2 线程机器上的机制探针结果，不是论文复现 |

对照实验要在 Linux 上跑。Windows（系统堆、MSVC `/MD`、clang-cl）是第二阶段的可移植性实验，不混进第一轮的 96 组里。
