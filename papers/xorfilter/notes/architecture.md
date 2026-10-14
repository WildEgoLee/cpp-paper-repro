# Xor Filter Architecture Notes

学习目标：静态成员测试。

## 核心对象

```
table[c]   // c ≈ 1.23 * n
```

## Ownership

- 表是唯一状态。
- 构建后不可变。

## Fast path / Slow path

- Fast：三次哈希 + XOR。
- Slow：构建时解方程。

## Invariants

1. 已插入的元素 XOR 等于指纹。
2. 空间约 1.23n。
3. 构建后不可变。

## Policy / Mechanism 分离

- Policy：指纹位数。
- Mechanism：三哈希 XOR。

## Concurrency boundary

查找是只读的；构建是单线程。

## State 位置

- 唯一状态：c 个槽。

## Data layout 理由

XOR 让三个槽的值组合成指纹。

## Failure modes

- 构建失败 → 重试哈希。
- 不支持插入/删除。

## 与 Bloom/Cuckoo 对比

| | 空间 | 动态 | 查找 |
|--|------|------|------|
| Bloom | 大 | 有 | 多位 |
| Cuckoo | 中 | 有 | 两桶 |
| Xor | 小 | 无 | 三哈希 |
