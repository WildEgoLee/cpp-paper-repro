# Robin Hood Hashing Architecture Notes

学习目标：有界方差的探测。

## 核心对象

```
Slot
  key
  psl          // probe sequence length
Table[n]
```

## Ownership

- 每个键占一个槽。
- 插入时，PSL 更大的键可以偷走 PSL 更小的键。

## Fast path / Slow path

- Fast：查找时沿探测序列走，遇到 PSL 更小的槽就可以停。
- Slow：插入时可能连续偷多个键。

## Invariants

1. 每个键的 PSL 等于它距离 home 槽的步数。
2. 探测序列上的 PSL 非减。
3. 方差有界（近满时仍然）。

## Policy / Mechanism 分离

- Policy：表大小、负载因子。
- Mechanism：PSL 比较 + 偷取。

## Concurrency boundary

单线程玩具；真实实现可以做细粒度锁。

## State 位置

- 键与 PSL：每个槽内。
- home 槽：哈希决定。

## Data layout 理由

PSL 保存在槽里，让插入时可以比较“谁更贫”。

## Failure modes

- 负载过高 → 探测链变长。
- 删除需要向后移动或标记 tombstone。

## 与 Cuckoo 对比

| | 冲突处理 | 查找 | 方差 |
|--|----------|------|------|
| Cuckoo | 踢到另一张表 | 最坏 O(1) | 无 |
| Robin Hood | 偷更富的 | 期望 O(1) | 有界 |
