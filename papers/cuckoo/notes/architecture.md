# Cuckoo Hashing Architecture Notes

学习目标：踢出链与最坏情况常数查找。

## 核心对象

```
T1[r], T2[r]     // 两张表
h1, h2           // 两个哈希
```

## Ownership

- 每个键只存在 T1[h1(x)] 或 T2[h2(x)] 之一。
- 插入时可能踢出旧键到它的另一个位置。

## Fast path / Slow path

- Fast：查找只看两个位置。
- Slow：插入时踢出链；循环时 rehash。

## Invariants

1. 每个键最多占两个候选位置之一。
2. 查找最多两次访存。
3. 负载足够低时，插入期望 O(1)。

## Policy / Mechanism 分离

- Policy：表大小、最大踢出次数。
- Mechanism：双表 + 踢出链。

## Concurrency boundary

单线程玩具；真实实现可以做细粒度锁。

## State 位置

- 键：两张表的某个槽。
- 哈希函数：固定或可重建。

## Data layout 理由

两个候选位置让查找最坏情况是常数，不需要链表或探测序列。

## Failure modes

- 循环 → 无限踢出，需要 rehash。
- 负载过高 → 循环概率升高。

## 与 Bloom 对比

| | 查找 | 假阳性 | 空间 |
|--|------|--------|------|
| Bloom | 概率 | 有 | 固定 |
| Cuckoo | 最坏 O(1) | 无 | O(n) |
