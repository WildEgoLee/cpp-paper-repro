# B+ Tree Architecture Notes

学习目标：有序索引与范围扫描。

## 核心对象

```
Internal node   // separator keys + child pointers
Leaf node       // all key-value pairs + next pointer
Leaf chain      // sorted linked list of leaves
```

## Ownership

- 所有数据只存在叶子。
- 内部节点只做导航。
- 叶子之间用 next 指针保持有序。

## Fast path / Slow path

- Fast：等值查询（根到叶）。
- Slow：叶子满时分裂并向上提升键。

## Invariants

1. 所有叶子在同一层。
2. 叶子键有序，且通过 next 指针连接。
3. 内部节点的 separator 正确分割子树范围。

## Policy / Mechanism 分离

- Policy：页大小、最小占用率。
- Mechanism：分裂、合并、叶子链表。

## Concurrency boundary

单线程玩具；真实实现需要页级锁或 latch coupling。

## State 位置

- 键值：叶子页。
- 导航：内部页。

## Data layout 理由

叶子链表让范围扫描变成顺序 I/O，而不是重复根到叶。

## Failure modes

- 分裂后忘记更新父节点 → 搜索丢键。
- 叶子链表断开 → 范围扫描缺失。

## 与 LSM 对比

| | 写 | 范围扫 | 空间放大 |
|--|----|--------|----------|
| LSM | 追加 | 合并多层 | 有（compaction） |
| B+ Tree | 原地更新 | 叶子链表 | 低 |
