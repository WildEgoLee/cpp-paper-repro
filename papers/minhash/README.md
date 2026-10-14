# MinHash

Andrei Broder. *On the resemblance and containment of documents*. SEQUENCES 1997.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入集合相似度原语：

```
signature[k] = min hash under each of k permutations

similarity = fraction of positions where signatures agree
P(agree) = Jaccard(A, B)
```

最小哈希相同的概率等于 Jaccard。与频率/基数估计完全不同。

## 学习目标

- 最小哈希
- 为什么概率等于 Jaccard
- 签名一致比例
- 与精确交集的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：多哈希最小值签名
- 探针：两个集合，估计与真实 Jaccard 接近
