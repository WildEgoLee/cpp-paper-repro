# Bloom Filter Architecture Notes

学习目标：概率成员测试。

## 核心对象

```
bit array[m]        // 初始全 0
hash_functions[k]   // 映射到 [0, m)
```

## Ownership

- 位数组是唯一状态。
- 没有元素副本；成员信息被哈希进位模式。

## Fast path / Slow path

- Fast：查询时某个位为 0 → 立即返回不存在。
- Slow：所有位都为 1 → 可能假阳性，需要二次确认。

## Invariants

1. 插入过的元素，查询一定返回 true（无假阴性）。
2. 查询返回 false 时，元素一定不存在。
3. 假阳性率随 n 增加而升高。

## Policy / Mechanism 分离

- Policy：m 与 k 的选择（目标假阳性率）。
- Mechanism：位数组 + 多哈希。

## Concurrency boundary

位数组写入是原子的；读不需要同步。

## State 位置

- 唯一状态：m 位的位数组。
- 没有元素列表。

## Data layout 理由

位数组让空间与元素数无关，只与期望假阳性率有关。

## Failure modes

- 假阳性：未插入的元素哈希到已有的 1。
- 过度插入 → 假阳性率接近 1。
- 不支持删除（标准 Bloom）。

## 与哈希表对比

| | 空间 | 假阳性 | 假阴性 | 删除 |
|--|------|--------|--------|------|
| 哈希表 | O(n) | 无 | 无 | 有 |
| Bloom | O(1) | 有 | 无 | 无 |
