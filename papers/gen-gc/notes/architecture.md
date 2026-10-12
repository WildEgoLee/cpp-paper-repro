# Generational GC Architecture Notes

学习目标：分代假说与写屏障。

## 核心对象

```
Nursery          // 幼年代，频繁 minor GC
Old generation   // 老年代，偶尔 major GC
Card table       // 写屏障记录的 dirty cards
Remembered set   // 跨代指针（old → young）
```

## Ownership

- 新分配进 nursery。
- 存活的 nursery 对象被促进到 old。
- 写屏障拥有“哪些老对象指向幼对象”的知识。

## Fast path / Slow path

- Fast：nursery 分配（bump pointer）。
- Slow：minor GC（复制存活对象）+ 写屏障检查。

## Invariants

1. Minor GC 只需扫描 nursery + remembered set，不需扫全堆。
2. 任何 old → young 的指针必须被写屏障记录。
3. 促进后对象不再参与 minor GC。

## Policy / Mechanism 分离

- Policy：促进阈值（存活 N 次后促进）、nursery 大小。
- Mechanism：写屏障、card table、复制收集。

## Concurrency boundary

写屏障必须在 mutator 写指针时同步更新 remembered set。

## State 位置

- 对象：nursery 或 old。
- 跨代指针：card table / remembered set。

## Data layout 理由

Nursery 用复制收集，存活对象被紧凑到一边，死亡对象被丢弃。
Card table 让写屏障只需 O(1) 标记。

## Failure modes

- 忘记写屏障 → minor GC 漏掉 old → young 指针，对象被错误回收。
- 促进过多 → old 被短命对象占满。
- Nursery 太大 → minor GC 停顿变长。

## 与其他内存管理对比

| | 回收单位 | 开销 |
|--|----------|------|
| malloc/free | 手动 | 无 |
| RCU | 宽限期 | 读端低 |
| Generational GC | 幼年代 | 写屏障 |
