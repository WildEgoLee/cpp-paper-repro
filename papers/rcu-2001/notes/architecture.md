# RCU Architecture Notes

学习目标：理解 deferred reclamation 与 grace period 这种并发原语。

## 核心对象

```
RcuDomain          // 管理宽限期
ReaderSection      // 读者临界区（无锁）
QuiescentState     // 线程进入静止点
GracePeriod        // 所有读者都已经历过静止
Callback / Free    // 宽限期后执行的回收
```

## Ownership

- 旧版本数据的所有权在宽限期结束前属于可能仍在读它的读者。
- 写者不能立即 free；必须等待 grace period。
- RcuDomain 拥有待回收列表。

## Fast path / Slow path

- Fast（读）：rcu_read_lock / rcu_read_unlock 几乎零开销（通常只是禁用抢占或记录 nesting）。
- Slow（写）：复制新版本 → 原子发布 → synchronize_rcu 等待宽限期 → free 旧版本。

## Invariants

1. 读者在 critical section 内看到的指针不会被释放。
2. 宽限期结束后，所有之前的读者都已经离开 critical section。
3. 新读者看到的是新版本（或更新之后的版本）。

## Policy / Mechanism 分离

- Policy：何时视为 quiescent（上下文切换、显式 rcu_quiescent_state、或读者离开）。
- Mechanism：如何检测宽限期结束（全局计数器、per-thread 快照、序列号）。

## Concurrency boundary

- 读者之间无同步。
- 读者与写者之间通过 grace period 协调，而非锁。
- 写者之间通常仍需要锁或其他同步。

## State 位置

- Per-thread：是否在 reader section、当前 grace period 序列号。
- Global：当前 grace period 计数器、待回收队列。

## Data layout 理由

读路径必须极快，因此把同步开销推迟到写路径的稀有操作。

## Failure modes

- 读者泄漏（未退出 critical section）→ 宽限期永不结束。
- 在 critical section 内休眠 / 阻塞 → 宽限期延迟。
- 错误地在宽限期前 free → use-after-free。

## 与其他技术对比

| 技术 | 读开销 | 写开销 | 回收时机 |
|------|--------|--------|----------|
| Mutex | 高 | 高 | 立即 |
| RWLock | 中 | 高 | 立即 |
| Hazard Pointer | 低 | 中 | 显式保护 |
| RCU | 极低 | 中（等待） | 宽限期后 |
