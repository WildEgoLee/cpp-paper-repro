# Count-Min Sketch Architecture Notes

学习目标：频率估计。

## 核心对象

```
counters[d][w]   // d 行，w 列
hash[d]          // 每行一个哈希
```

## Ownership

- 计数器是唯一状态。
- 没有元素副本；频率被压缩进计数器。

## Fast path / Slow path

- Fast：更新时每行 +1。
- Slow：查询时取 d 个计数器的最小值。

## Invariants

1. 估计 >= 真实频率（不低估）。
2. 取最小值用碰撞最少的行。
3. 空间与元素数无关。

## Policy / Mechanism 分离

- Policy：d 与 w（误差概率与幅度）。
- Mechanism：多行哈希 + 取 min。

## Concurrency boundary

计数器更新是原子的；查询可以并发。

## State 位置

- 唯一状态：d * w 个计数器。

## Data layout 理由

多行让至少有一行碰撞少，取 min 得到近似上界。

## Failure modes

- 碰撞 → 过估计。
- 不支持删除（标准 CMS）。

## 与精确计数对比

| | 空间 | 估计 | 删除 |
|--|------|------|------|
| 精确 | O(n) | 精确 | 有 |
| CMS | O(1) | 过估计 | 无 |
