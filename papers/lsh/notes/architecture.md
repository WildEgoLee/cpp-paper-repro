# LSH Architecture Notes

学习目标：候选生成。

## 核心对象

```
bands[b][r]   // 签名分成 b 带，每带 r 行
```

## Ownership

- 签名是输入。
- 桶是候选的集合。

## Fast path / Slow path

- Fast：哈希一带到桶。
- Slow：检查候选对的真实相似度。

## Invariants

1. 至少一带相同 → 候选。
2. 阈值约等于 (1/b)^(1/r)。
3. b 大 → 阈值低；r 大 → 阈值高。

## Policy / Mechanism 分离

- Policy：b 与 r（阈值）。
- Mechanism：带状哈希。

## Concurrency boundary

带可以独立哈希；候选检查可以并发。

## State 位置

- 唯一状态：b 个哈希桶。

## Data layout 理由

带状让相似对更容易碰撞，不相似对更不容易。

## Failure modes

- 假阳性：不相似对成为候选。
- 假阴性：相似对未成为候选。

## 与全量比较对比

| | 比较次数 | 准确度 |
|--|----------|--------|
| 全量 | O(n^2) | 精确 |
| LSH | O(候选) | 近似 |
