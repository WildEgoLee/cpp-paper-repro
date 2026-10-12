# Work-Stealing Architecture Notes

学习目标：任务并行运行时如何用 deque + steal 做负载均衡。

## 核心对象

```
Worker
  deque          // double-ended queue of ready tasks
  bottom         // owner pushes/pops here (LIFO)
  top            // thieves steal from here (FIFO)
Task / Closure   // 可被 steal 的工作单元
```

## Ownership

- 每个 Worker 拥有自己的 deque。
- 任务在被 steal 前属于 owner；steal 后 ownership 转移。
- 没有全局任务队列（避免中心瓶颈）。

## Fast path / Slow path

- Fast：本地 bottom push/pop（几乎无同步）。
- Slow：deque 空 → 随机选 victim → 从其 top steal。

## Invariants

1. Owner 只操作 bottom，thief 只操作 top。
2. 同一任务不会被两个 worker 同时执行。
3. Work-first：本地执行优先，steal 是稀有路径。

## Policy / Mechanism 分离

- Policy：选哪个 victim（随机、轮询、最近）。
- Mechanism：deque 的并发 push/pop/steal 协议。

## Concurrency boundary

- Bottom 操作通常无锁或轻量同步。
- Top steal 需要与 owner 协调（CAS 或锁）。

## State 位置

- 就绪任务：各 worker 的 deque。
- 正在执行的任务：worker 的当前帧。

## Data layout 理由

本地 LIFO 保持缓存局部性；从对端 steal 使被偷的任务更像 BFS，有利于负载均衡。

## Failure modes

- 所有 worker 同时空 → 程序结束或死锁（需终止检测）。
- Steal 冲突过多 → 缓存行争用。
- 任务粒度太细 → 调度开销压过工作。

## 理论界

对 fully-strict 计算：

\[ T_P = O\left(\frac{T_1}{P} + T_\infty\right) \]

空间 \( S_P \le S_1 P \)。
