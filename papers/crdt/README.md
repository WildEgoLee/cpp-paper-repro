# CRDT (Conflict-free Replicated Data Type)

State-based and operation-based conflict-free types.

Classic references:
- Shapiro et al., *Conflict-free Replicated Data Types* (SSS 2011)
- Shapiro et al., *A comprehensive study of Convergent and Commutative Replicated Data Types* (INRIA RR-7506)

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入无冲突复制原语：

```
G-Counter
  payload: int[n]          // one slot per replica
  increment():  P[myId]++
  merge(other): P[i] = max(P[i], other.P[i])
  value():      sum(P)
```

merge 是可交换、结合、幂等的，所以不需要协调就能收敛。与 Raft 的强一致性完全不同。

## 学习目标

- State-based vs operation-based
- G-Counter 的 max-merge
- 为什么可交换 + 幂等 = 无冲突
- 与 consensus 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：G-Counter
- 探针：两个副本独立递增后 merge，值正确
