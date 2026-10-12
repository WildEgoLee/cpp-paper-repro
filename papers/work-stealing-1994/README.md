# Work-Stealing Scheduler

Robert D. Blumofe, Charles E. Leiserson. *Scheduling Multithreaded Computations by Work Stealing*. FOCS 1994 / JACM 1999.

Implemented in Cilk.

- Paper: https://dl.acm.org/doi/10.1145/324133.324234
- Cilk runtime: classic MIT implementation

**Status**: architecture-extraction (learning-oriented).

## Why this paper

引入任务并行调度原语：

```
Worker
 ├─ local deque (bottom = owner, top = thieves)
 └─ when empty → steal from random victim's top
```

Work-first 原则：优化本地执行，把开销推迟到 steal 发生时。

与 RCU（宽限期）、LSM（存储）完全不同。

## 学习目标

- Per-worker deque（底部 push/pop，顶部 steal）
- Work-first：spawn 只暴露 continuation，不立即创建线程
- 随机 victim 选择
- 理论界：T_P = O(T_1/P + T_∞)

## 第一轮

- `notes/architecture.md`
- 极简 model：deque + 多 worker + 随机 steal
- 探针：空 deque 时能从其他 worker 偷到任务
