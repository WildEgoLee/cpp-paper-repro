# Count-Min Sketch

Graham Cormode, S. Muthukrishnan. *An Improved Data Stream Summary: The Count-Min Sketch and its Applications*. Journal of Algorithms 2005.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入频率估计原语：

```
counters[d][w]
d independent hash functions

add(x):   for each row, ++counters[row][h_row(x)]
estimate(x): min over rows of counters[row][h_row(x)]
```

估计永不低估。取最小值用最少碰撞的行。与 HLL 的基数估计完全不同。

## 学习目标

- 多行哈希
- 为什么取最小值
- 过估计不低估
- 与精确计数的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：多行计数器 + 取 min
- 探针：插入后估计，估计 >= 真实频率
