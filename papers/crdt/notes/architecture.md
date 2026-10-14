# CRDT Architecture Notes

学习目标：无冲突复制。

## 核心对象

```
G-Counter
  payload[n]     // one counter per replica
  merge          // pointwise max
  value          // sum
```

## Ownership

- 每个副本只写自己的 slot。
- merge 不需要协调；取 max 即可。
- 最终值是所有 slot 之和。

## Fast path / Slow path

- Fast：本地 increment。
- Slow：后台传播 state 并 merge。

## Invariants

1. merge 是可交换、结合、幂等的。
2. 任意两个副本收到相同的更新集合后，值相等。
3. 每个副本只增加自己的 slot。

## Policy / Mechanism 分离

- Policy：多久同步一次 state。
- Mechanism：pointwise max merge。

## Concurrency boundary

副本之间无需协调；只要最终都收到同一些更新。

## State 位置

- 每个副本保持完整 payload。
- 同步时传输整个 payload（state-based）。

## Data layout 理由

数组按副本 ID 索引，让 max 可以独立作用于每个 slot。

## Failure modes

- 丢失部分更新 → 值偏低，但不会分裂。
- 不能表达“减少”（G-Counter 限制）。

## 与 Raft 对比

| | 一致性 | 可用性 | 协调 |
|--|--------|--------|------|
| Raft | 强 | 多数派 | 需要 |
| CRDT | 最终 | 高 | 不需要 |
