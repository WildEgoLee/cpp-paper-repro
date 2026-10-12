# Persistent Data Structures Architecture Notes

学习目标：路径复制与结构共享。

## 核心对象

```
Node
  key, value
  left, right     // 指向其他 Node
Version
  root            // 指向该版本的根
```

## Ownership

- 节点一旦创建就不可变。
- 新版本拥有新的 root；未改变的子树被多个版本共享。
- 无人拥有“当前”版本；所有版本平等。

## Fast path / Slow path

- Fast：查询只读不变的树。
- Slow：更新需要复制 O(log n) 个节点。

## Invariants

1. 节点创建后不可修改。
2. 新 root 与旧 root 共享所有未改变的子树。
3. 任何历史版本都可独立查询。

## Policy / Mechanism 分离

- Policy：保留多少版本、何时回收。
- Mechanism：路径复制、结构共享。

## Concurrency boundary

不可变性让读取完全无锁；写只创建新节点。

## State 位置

- 数据：不可变节点图。
- 版本：root 指针数组。

## Data layout 理由

路径复制使单次更新只分配 O(log n) 个节点，而非整棵树。

## Failure modes

- 版本过多且无回收 → 内存泄漏。
- 误改旧节点 → 破坏历史版本。

## 与可变结构对比

| | 更新 | 历史查询 | 并发读 |
|--|------|----------|--------|
| Mutable BST | in-place | 无 | 需锁 |
| Persistent BST | path copy | 有 | 无锁 |
