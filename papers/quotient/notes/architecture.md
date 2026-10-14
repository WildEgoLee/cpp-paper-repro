# Quotient Filter Architecture Notes

学习目标：可删除的成员测试。

## 核心对象

```
slots[m]   // 每个槽存余数
```

## Ownership

- 余数是唯一状态。
- 商决定主槽。

## Fast path / Slow path

- Fast：查找运行。
- Slow：插入时移动余数。

## Invariants

1. 同商的余数连续存储。
2. 查找只扫描运行。
3. 删除去掉余数。

## Policy / Mechanism 分离

- Policy：商位数、余数位数。
- Mechanism：运行。

## Concurrency boundary

槽更新是原子的；查找可以并发。

## State 位置

- 唯一状态：m 个槽。

## Data layout 理由

运行让同商的余数连续，提高缓存局部性。

## Failure modes

- 运行太长 → 查找变慢。
- 删除时移动错误 → 假阴性。

## 与 Cuckoo Filter 对比

| | 局部性 | 删除 | 查找 |
|--|--------|------|------|
| Cuckoo | 两桶 | 有 | 固定 |
| Quotient | 运行 | 有 | 扫描 |
