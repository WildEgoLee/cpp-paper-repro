# Skip List Architecture Notes

学习目标：概率平衡。

## 核心对象

```
Node
  key
  forward[level]   // 每层一个前向指针
Head
  forward[MaxLevel]
```

## Ownership

- 每个节点的高度在插入时随机决定，之后不变。
- 插入/删除只修改邻居节点的 forward 指针。

## Fast path / Slow path

- Fast：从高层开始搜索，跳过大段。
- Slow：高层稀疏时退回低层。

## Invariants

1. 每一层都是有序的。
2. 高层节点是低层的子集。
3. 随机高度使期望高度为 O(log N)。

## Policy / Mechanism 分离

- Policy：p（晋升概率）、MaxLevel。
- Mechanism：多层 forward 指针、局部拼接。

## Concurrency boundary

单线程玩具；真实实现可以做无锁插入（因为修改是局部的）。

## State 位置

- 键与指针：每个节点内。
- 当前最高层：全局。

## Data layout 理由

高层指针跳得远，让搜索期望步数为 O(log N)，而不需要维护平衡信息。

## Failure modes

- 随机高度全部为 1 → 退化为链表。
- 忘记更新某一层的 forward → 搜索丢键。

## 与 B+ Tree 对比

| | 平衡 | 插入 | 范围扫 |
|--|------|------|--------|
| B+ Tree | 确定性分裂 | 可能级联 | 叶子链表 |
| Skip List | 概率 | 局部拼接 | 底层链表 |
