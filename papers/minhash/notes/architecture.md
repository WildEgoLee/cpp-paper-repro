# MinHash Architecture Notes

学习目标：集合相似度。

## 核心对象

```
signature[k]   // 每个置换下的最小哈希
```

## Ownership

- 签名是集合的压缩表示。
- 没有元素副本；只保留最小值。

## Fast path / Slow path

- Fast：更新签名（取 min）。
- Slow：比较两个签名的一致比例。

## Invariants

1. 单位置一致概率 = Jaccard。
2. 多位置平均抑制方差。
3. 空间与集合大小无关。

## Policy / Mechanism 分离

- Policy：k（签名长度）。
- Mechanism：最小哈希。

## Concurrency boundary

签名可以独立计算；比较可以并发。

## State 位置

- 唯一状态：k 个最小哈希值。

## Data layout 理由

只保留最小值，让一致概率等于 Jaccard。

## Failure modes

- 哈希碰撞 → 估计偏差。
- k 太小 → 方差大。

## 与精确交集对比

| | 空间 | 准确度 | 速度 |
|--|------|--------|------|
| 精确 | O(n) | 精确 | 慢 |
| MinHash | O(k) | 近似 | 快 |
