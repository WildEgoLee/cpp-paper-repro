# Count Sketch

Moses Charikar, Kevin Chen, Martin Farach-Colton. *Finding Frequent Items in Data Streams*. ICALP 2002.

**Status**: architecture-extraction (learning-oriented).

## Why this topic

引入无偏频率估计原语：

```
counters[d][w]
sign hash g_row(x) in {+1, -1}

add(x):   for each row, counters[row][h(x)] += g(x)
estimate(x): median over rows of g(x) * counters[row][h(x)]
```

符号哈希让碰撞期望为零。取中位数抑制方差。与 CMS 的过估计完全不同。

## 学习目标

- 符号哈希
- 为什么无偏
- 中位数为什么比平均好
- 与 CMS 的对比

## 第一轮

- `notes/architecture.md`
- 极简 model：符号哈希 + 中位数
- 探针：插入后估计，与真实频率接近
