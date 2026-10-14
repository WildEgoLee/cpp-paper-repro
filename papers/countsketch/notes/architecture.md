# Count Sketch Architecture Notes

学习目标：无偏频率估计。

## 核心对象

```
counters[d][w]
g[d]   // sign hash per row
```

## Ownership

- 计数器是唯一状态。
- 没有元素副本；频率被符号混合进计数器。

## Fast path / Slow path

- Fast：更新时每行加/减。
- Slow：查询时取中位数。

## Invariants

1. 单行估计期望等于真实频率。
2. 中位数抑制方差。
3. 空间与元素数无关。

## Policy / Mechanism 分离

- Policy：d 与 w。
- Mechanism：符号哈希 + 中位数。

## Concurrency boundary

计数器更新是原子的；查询可以并发。

## State 位置

- 唯一状态：d * w 个计数器。

## Data layout 理由

符号让碰撞正负抵消，中位数去掉极端值。

## Failure modes

- 碰撞 → 方差。
- 需要多行才能用中位数。

## 与 CMS 对比

| | 估计 | 偏差 |
|--|------|------|
| CMS | 过估计 | 有 |
| Count Sketch | 无偏 | 无 |
