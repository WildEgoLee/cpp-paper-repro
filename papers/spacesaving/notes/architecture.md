# Space-Saving Architecture Notes

学习目标：重击者追踪。

## 核心对象

```
counters[k]   // (item, count)
```

## Ownership

- 只追踪 k 个元素。
- 新元素到达时，替换最小计数的元素。

## Fast path / Slow path

- Fast：已追踪的元素直接 +1。
- Slow：找最小计数并替换。

## Invariants

1. 最小计数 <= N/k。
2. 频率 > N/k 的元素不会被淘汰。
3. 估计误差有界。

## Policy / Mechanism 分离

- Policy：k（计数器数）。
- Mechanism：替换最小。

## Concurrency boundary

单线程玩具；真实实现可以用堆维护最小。

## State 位置

- 唯一状态：k 个 (item, count)。

## Data layout 理由

固定 k 个槽，让重击者不会被频率低的元素淘汰。

## Failure modes

- 频率接近阈值的元素可能被淘汰。
- 不支持删除。

## 与 CMS 对比

| | 追踪 | 估计 | 重击者 |
|--|------|------|--------|
| CMS | 所有（概率） | 过估计 | 可能漏 |
| Space-Saving | 固定 k | 替换 | 保留 |
