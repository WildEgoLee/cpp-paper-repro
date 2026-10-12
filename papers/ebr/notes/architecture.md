# Epoch-based Reclamation Architecture Notes

学习目标：基于 epoch 的安全回收。

## 核心对象

```
Global epoch          // 单调递增
Per-thread announce   // 当前线程观察到的 epoch
Limbo lists[3]        // 每个 epoch 一个待释放列表
```

## Ownership

- 节点被 retire 后进入当前 epoch 的 limbo list。
- 只有当所有线程都已观察到更新的 epoch 后，旧 epoch 的 limbo 才能释放。
- 线程不在 critical section 时不阻碍推进。

## Fast path / Slow path

- Fast：enter 时只读全局 epoch 并写宣告。
- Slow：推进 epoch 时需要检查所有线程的宣告。

## Invariants

1. 线程在 critical section 内最多比全局 epoch 落后 1。
2. 在 epoch e 被 retire 的节点，只有在 epoch 推进到 e+2 后才能释放。
3. 不在 critical section 的线程不会阻碍回收。

## Policy / Mechanism 分离

- Policy：多久尝试推进一次 epoch。
- Mechanism：宣告、limbo list、三 epoch 轮转。

## Concurrency boundary

- 宣告是原子写。
- 推进 epoch 需要看到所有宣告。

## State 位置

- 全局 epoch：共享。
- 宣告：每线程一个。
- Limbo：每 epoch 一个列表。

## Data layout 理由

三个 limbo list 轮转使用，因为线程最多落后一个 epoch。

## Failure modes

- 线程死在 critical section → epoch 无法推进，内存泄漏。
- 忘记 enter/exit → 过早释放或泄漏。

## 与其他回收对比

| | 保护粒度 | 停顿线程影响 |
|--|----------|--------------|
| Hazard pointer | 每指针 | 有界 |
| RCU | 宽限期 | 可能长 |
| EBR | 整次操作 | 死在临界区则无界 |
