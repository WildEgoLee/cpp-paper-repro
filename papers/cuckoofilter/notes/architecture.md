# Cuckoo Filter Architecture Notes

学习目标：可删除的成员测试。

## 核心对象

```
buckets[n]   // 每个桶存指纹
```

## Ownership

- 指纹是唯一状态。
- 没有元素副本；只存指纹。

## Fast path / Slow path

- Fast：查找两个桶。
- Slow：踢出链。

## Invariants

1. 查找只检查两个桶。
2. 第二个桶由第一个 XOR 指纹哈希得到。
3. 删除去掉指纹。

## Policy / Mechanism 分离

- Policy：桶数、指纹位数。
- Mechanism：指纹 + XOR 派生。

## Concurrency boundary

桶更新是原子的；查找可以并发。

## State 位置

- 唯一状态：n 个桶。

## Data layout 理由

指纹让空间小，XOR 让第二个桶可以从指纹派生。

## Failure modes

- 踢出链太长 → 插入失败。
- 删除未插入的项 → 可能误删。

## 与 Bloom Filter 对比

| | 删除 | 空间 | 查找 |
|--|------|------|------|
| Bloom | 无 | 位图 | 多位 |
| Cuckoo | 有 | 指纹 | 两桶 |
