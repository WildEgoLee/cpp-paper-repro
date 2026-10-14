# Locality-Sensitive Hashing (LSH)

Piotr Indyk, Rajeev Motwani. *Approximate Nearest Neighbors: Towards Removing the Curse of Dimensionality*. STOC 1998.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入带状分桶原语：

```
signature[b * r]

candidate if at least one band is identical
P(candidate) = 1 - (1 - s^r)^b
```

b 和 r 控制阈值。与 MinHash 的估计完全不同。

## 学习目标

- 带状分割
- 为什么至少一带相同
- 阈值与 b/r 的关系
- 与全量比较的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：带状哈希
- 探针：相似对成为候选，不相似对不成
