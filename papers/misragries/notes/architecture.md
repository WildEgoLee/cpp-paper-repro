# Misra-Gries Architecture Notes

学习目标：频繁项。

## 核心对象

```
counters[k-1]   // (item, count)
```

## Ownership

- 只追踪 k-1 个元素。
- 新元素到达且已满时，全体计数器减一。

## Fast path / Slow path

- Fast：已追踪的元素直接 +1。
- Slow：全体减一，丢弃零。

## Invariants

1. 频率 > n/k 的元素不会被丢弃。
2. 估计 <= 真实频率。
3. 减一次数有界。

## Policy / Mechanism 分离

- Policy：k。
- Mechanism：全体减一。

## Concurrency boundary

单线程玩具。

## State 位置

- 唯一状态：k-1 个 (item, count)。

## Data layout 理由

固定槽，让频繁项不会被低频元素淘汰。

## Failure modes

- 频率接近阈值的元素可能被丢弃。
- 不支持删除。

## 与 Space-Saving 对比

| | 新元素 | 估计 |
|--|--------|------|
| Space-Saving | 替换最小 | 过估计 |
| Misra-Gries | 全体减一 | 低估计 |
