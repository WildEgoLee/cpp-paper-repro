# HyperLogLog Architecture Notes

学习目标：基数估计。

## 核心对象

```
registers[m]   // 每个寄存器记录见过的最大前导零数
```

## Ownership

- 寄存器是唯一状态。
- 没有元素副本；基数信息被压缩成前导零。

## Fast path / Slow path

- Fast：更新寄存器（取 max）。
- Slow：估计时计算调和平均。

## Invariants

1. 前导零数更多 → 估计基数更大。
2. 调和平均抑制异常大值。
3. 空间与元素数无关。

## Policy / Mechanism 分离

- Policy：寄存器数 m、哈希位数。
- Mechanism：前导零 + 调和平均。

## Concurrency boundary

寄存器更新是原子的；估计可以并发。

## State 位置

- 唯一状态：m 个小整数寄存器。

## Data layout 理由

每个寄存器只存最大前导零，让空间与基数无关。

## Failure modes

- 小基数时估计有偏差（需要线性计数修正）。
- 哈希碰撞 → 大基数估计饱和。

## 与精确计数对比

| | 空间 | 准确度 | 合并 |
|--|------|--------|------|
| 精确 | O(n) | 精确 | 难 |
| HLL | O(1) | 近似 | 易（取 max） |
