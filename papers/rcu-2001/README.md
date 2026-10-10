# Read-Copy Update (RCU)

Paul E. McKenney et al. *Read-Copy Update*. OLS 2001 and subsequent work.

- Classic paper: https://www.kernel.org/doc/ols/2001/read-copy.pdf
- User-space RCU: Desnoyers et al., "User-Level Implementations of Read-Copy Update"
- Modern: Linux kernel RCU, liburcu, C++26 P2545

**Status**: architecture-extraction (learning-oriented).

## Why this paper

引入一种全新的 architecture primitive：**deferred reclamation via grace periods**。

与现有四篇不同：

- 不是 allocator / compiler / binary layout / VM remapping
- 是 read-mostly 并发模型：读者无锁，写者复制-更新-等待宽限期

## 学习目标

```
reader critical section
    ↓
quiescent state
    ↓
grace period
    ↓
safe reclamation
```

要掌握：

- 读路径零同步开销
- 写路径两阶段更新（publish new + wait for old readers）
- 宽限期检测（per-thread quiescent state）
- 与锁、hazard pointer、epoch-based reclamation 的差异

## 第一轮（学习导向）

- 写 `notes/architecture.md`
- 实现 toy model：简单链表 + RCU 更新
- 探针：多线程读者 + 写者，验证宽限期后旧节点可安全释放
- 不追求完整 liburcu 或内核 RCU 复现
